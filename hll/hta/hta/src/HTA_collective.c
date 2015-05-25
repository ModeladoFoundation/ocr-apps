#include <string.h>
#include <stdio.h>
#include "Tuple.h"
#include "HTA.h"
#include "Comm.h"
#include "pil.h"
#include "pil_mem.h"

void HTA_to_array_distributed(HTA *h, void* array)
{
  //Dummy function
  assert(0 && "Not implemented");
}

static void _get_shift_target(Tuple * target, Tuple * nd_idx, Tuple * nd_size, Tuple * direction)
{
    ASSERT(target->dim == nd_idx->dim && nd_idx->dim == nd_size->dim && nd_size->dim == direction->dim);

    for(int i = 0; i < target->dim; i++){
        int size = nd_size->values[i];
        int dir = direction->values[i] % size;
        dir = (dir < 0) ? (size + dir) : dir; // make dir be positive
        target->values[i] = (nd_idx->values[i] + dir) % size;
    }
}

static void _recursive_rerank(HTA *h, int* rank)
{
    if(h->height == 1) {
        h->rank = *rank;
        (*rank)++;
    }
    else {
        for(int i = 0; i < h->num_tiles; i++){
            _recursive_rerank(h->tiles[i], rank);
        }
    }
}

static void _change_home(HTA* h, int new_home)
{
    ASSERT(h);

    h->home = new_home;

    if(h->height == 1) {
        return;
    } else { // h->height > 1
        for(int i = 0; i < h->num_tiles; i++) {
            _change_home(h->tiles[i], new_home);
        }
    }
}

static void _recursive_setpid(HTA* h, int pid)
{
    ASSERT(h);

    h->pid = pid;

    if(h->height == 1) {
        return;
    } else { // h->height > 1
        for(int i = 0; i < h->num_tiles; i++) {
            _recursive_setpid(h->tiles[i], pid);
        }
    }
}

// FIXME: this implementation works for shared memory model only
void HTA_circshift(HTA *h, Tuple *dir) {
    ASSERT(h->dim == dir->dim);

    if(h->height == 1) // a leaf tile
        return;

    int dim = h->dim;
    HTA* newtiles[h->num_tiles]; // a temporary array for storing shifted pointers
    int  owner[h->num_tiles]; // to memorize the owner at certain index
    int  orig[h->num_tiles];

    Tuple iter;
    Tuple target;
    Tuple_iterator_begin(dim, 1, &iter); // an iterator that walks at 1st level only
    Tuple_init_zero(&target, dim);
    do {
        HTA* cur_t = HTA_pick_one_tile(h, &iter);
        _get_shift_target(&target, &iter, &h->tiling, dir);         // get the nd index of shift target position
        int target_idx = Tuple_nd_to_1d_index(&target, &h->tiling); // convert to 1d index
        //ASSERT(newtiles[target_idx] == NULL);                     // must not be written any value yet
        newtiles[target_idx] = cur_t;                               // store the pointer to the current subtree
        orig[target_idx] = cur_t->home;                             // memorize where the subtree is originated
    }while(Tuple_iterator_next(&h->tiling, &iter));

    if(CFG_get(CFG_ALLOW_SHARED_DATA)) {
        // overwrite original pointers
        for(int i = 0; i < h->num_tiles; i++)
        {
            owner[i] = h->tiles[i]->home; // memorize the home of the subtree
            h->tiles[i] = newtiles[i];    // replace the subtree
        }
    } else { // for SPMD
        int mypid = h->pid;
        // instead of sending the data of tiles, send the pointer to the subtree out
        gpp_t buf;
        pil_alloc(&buf, sizeof(HTA*));
        for(int i = 0; i < h->num_tiles; i++) { // NOTICE: owner collection must be in a separate loop,
                                                // otherwise it is possible for a race condition to happen
                                                // because some thread can enter the reranking before others do
            owner[i] = h->tiles[i]->home; // memorize the home of the subtree
        }

        for(int i = 0; i < h->num_tiles; i++) {
            if(owner[i] == mypid) { // new owner
                if(owner[i] != orig[i]) { // non-local
                    memcpy(buf.ptr, &newtiles[i], sizeof(HTA*));
                    comm_sendrecv(mypid, buf, orig[i], sizeof(HTA*));
                    memcpy(&newtiles[i], buf.ptr, sizeof(HTA*));
                    _recursive_setpid(newtiles[i], mypid); // since the whole subtree is taken from others, metadata ownership must be changed too
                }
            } else if (orig[i] == mypid) { // original owner, non-local
                memcpy(buf.ptr, &newtiles[i], sizeof(HTA*));
                comm_sendrecv(mypid, buf, owner[i], sizeof(HTA*));
                memcpy(&newtiles[i], buf.ptr, sizeof(HTA*));
                _recursive_setpid(newtiles[i], mypid); // since the whole subtree is taken from others, metadata ownership must be changed too
            }
            h->tiles[i] = newtiles[i];    // no matter if current process is or is not involved in communication, this must be performed
        }
        pil_free(buf);
    }

    // modify tile owner fields (Ownership changes)
    for(int i = 0; i < h->num_tiles; i++) {
        _change_home(h->tiles[i], owner[i]);
    }

    // traverse the HTA and update the rank value for they will be used to infer the location
    HTA_rerank(h);

    if(CFG_get(CFG_ALLOW_SHARED_DATA)) {
        HTA_barrier(h->pid); // barrier at the end to ensure the ownership is changed for all P
    }
}

void HTA_rerank(HTA *h)
{
    int rank = 0;
    _recursive_rerank(h, &rank);
}


// FIXME: for now it's a specialized operation to deal with FT only
// NOTICE: This should be OK to keep for SPMD since each thread has global pointer information already.
//         But a barrier is required before continuing since the ownership of data tiles is changed.
void HTA_transpose(HTA * xout, HTA * xin, int from_upper, int from_lower)
{
  int my_pid = xout->pid;
  // Assume that the tiling of xout is what we need already, so metadata doesn't have to be changed
  // Assume that upper level is 1D vector tiling and the same for lower level
  // Assume shared memory model for now

  ASSERT(from_upper != from_lower);

  int num_upper_tiles = xin->tiling.values[from_upper];
  int num_lower_tiles = xin->tiles[0]->tiling.values[from_lower];

  // FIXME: assuming regular tiles?
  ASSERT(num_upper_tiles == xout->tiles[0]->tiling.values[from_upper]
          && num_lower_tiles == xout->tiling.values[from_lower]);

  if(!CFG_get(CFG_OPT_ASYNC_SEND)) {
      // clone tiles
      for(int i = 0; i < num_upper_tiles; i++)
      {
        HTA * from_tile = xin->tiles[i];
        for(int j = 0; j < num_lower_tiles; j++)
        {
            HTA * to_tile = xout->tiles[j];
            HTA * from_leaf = from_tile->tiles[j];
            HTA * to_leaf = to_tile->tiles[i];
            int src_pid = from_leaf->home;
            int dest_pid = to_leaf->home;

            if(src_pid == dest_pid) { // same process
                // NOTICE: Performance optimization here for shared memory
                //         Swap pointer values instead of actually copying data
                if(my_pid == dest_pid) { // owner of data (this deals with fork-join case too)
                    // copy pointer instead of copy complete data
                    void * tmp = from_leaf->leaf.raw;
                    //from_leaf->leaf.raw = to_leaf->leaf.raw;
                    to_leaf->leaf.raw = tmp;
                }
            } else if(my_pid == src_pid) {
                //printf("(%d) ---> (%d)\n", my_pid, dest_pid);
                int num_elems = to_leaf->leaf.num_elem;
                void* from_leaf_ptr = HTA_get_ptr_raw_data(from_leaf);
                size_t sz = HTA_get_scalar_size(to_leaf);
                gpp_t buf = Alloc_recover_gpp(from_leaf_ptr);
                comm_send(my_pid, buf, dest_pid, sz * num_elems, sizeof(gpp_t));
            } else if (my_pid == dest_pid) {
                //printf("(%d) <--- (%d)\n", my_pid, src_pid);
                int num_elems = to_leaf->leaf.num_elem;
                size_t sz = HTA_get_scalar_size(to_leaf);
                gpp_t buf = Alloc_recover_gpp(HTA_get_ptr_raw_data(to_leaf));
                comm_recv(my_pid, buf, src_pid, sz * num_elems, sizeof(gpp_t));
            }
#if ENABLE_PREPACKING
            to_tile->prepacked = 0;
#endif
        }
#if ENABLE_PREPACKING
        from_tile->prepacked = 0;
#endif
      }
  }
  else {
      // Post asynchronous sends first
      for(int i = 0; i < num_upper_tiles; i++)
      {
        HTA * from_tile = xin->tiles[i];
        for(int j = 0; j < num_lower_tiles; j++)
        {
            HTA * to_tile = xout->tiles[j];
            HTA * from_leaf = from_tile->tiles[j];
            HTA * to_leaf = to_tile->tiles[i];
            int src_pid = from_leaf->home;
            int dest_pid = to_leaf->home;

            if(src_pid == dest_pid) { // same process
                // NOTICE: Performance optimization here for shared memory
                //         Swap pointer values instead of actually copying data
                if(my_pid == dest_pid) { // owner of data (this deals with fork-join case too)
                    // copy pointer instead of copy complete data
                    void * tmp = from_leaf->leaf.raw;
                    //from_leaf->leaf.raw = to_leaf->leaf.raw;
                    to_leaf->leaf.raw = tmp;
                }
            } else if(my_pid == src_pid) {
                //printf("(%d) ---> (%d)\n", my_pid, dest_pid);
                int num_elems = to_leaf->leaf.num_elem;
                void* from_leaf_ptr = HTA_get_ptr_raw_data(from_leaf);
                size_t sz = HTA_get_scalar_size(to_leaf);
                gpp_t buf = Alloc_recover_gpp(from_leaf_ptr);
                comm_send(my_pid, buf, dest_pid, sz * num_elems, sizeof(gpp_t));
            }
#if ENABLE_PREPACKING
            to_tile->prepacked = 0;
#endif
        }
#if ENABLE_PREPACKING
        from_tile->prepacked = 0;
#endif
      }
      // Start receiving
      for(int i = 0; i < num_upper_tiles; i++)
      {
        HTA * from_tile = xin->tiles[i];
        for(int j = 0; j < num_lower_tiles; j++)
        {
            HTA * to_tile = xout->tiles[j];
            HTA * from_leaf = from_tile->tiles[j];
            HTA * to_leaf = to_tile->tiles[i];
            int src_pid = from_leaf->home;
            int dest_pid = to_leaf->home;

            if (src_pid != dest_pid && my_pid == dest_pid) {
                //printf("(%d) <--- (%d)\n", my_pid, src_pid);
                int num_elems = to_leaf->leaf.num_elem;
                size_t sz = HTA_get_scalar_size(to_leaf);
                gpp_t buf = Alloc_recover_gpp(HTA_get_ptr_raw_data(to_leaf));
                comm_recv(my_pid, buf, src_pid, sz * num_elems, sizeof(gpp_t));
            }
        }
      }
  }
  // no need to change metadata information because it's assumed to be allocated correctly for the output HTA *
}

