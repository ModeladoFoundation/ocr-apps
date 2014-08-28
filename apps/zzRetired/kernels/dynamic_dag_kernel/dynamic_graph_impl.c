//my own response to avl,treap etc.
//for emphermal sets (sets that have very short life)
//the idea is to keep operations really simple
//at the cost of twice  storage.
//left-right rotation is avoided by hashing the key to left-or-right child
//mixing of  hash and binary-tree is necessary --
//since we can never estimate  size of input-elements
//its both an hash and a set (haset or sash)



//the code is modification of Tree.h
//That generates dynamic social network

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "assert.h"

#include "dynamic_graph_impl.h"


#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}
extern uint32_t hashlittle( const void *key, size_t length, uint32_t initval);






struct graphtree_idx* construct_graphtree_idx()
{
  struct graphtree_idx* gptr = (struct graphtree_idx*)malloc(sizeof(struct graphtree_idx));
  gptr->left_type = -1; //undef
  gptr->right_type = -1; //undef
  gptr->parent = 0;
  return gptr;
}



struct adjtree_idx* construct_adjtree_idx()
{
  struct adjtree_idx* aptr = (struct adjtree_idx*)malloc(sizeof(struct adjtree_idx));
  aptr->left_type = -1; //undef
  aptr->right_type = -1; //undef
  aptr->parent = 0;
  return aptr;
}


struct graphtree_leaf* construct_graphtree_leaf(struct graphtree_idx*p, uint32_t k, struct adjtree_idx* v)
{
  struct graphtree_leaf* glptr = (struct graphtree_leaf*)malloc(sizeof(struct graphtree_leaf));
  glptr->parent = p;
  glptr->nodeid  = k;
  glptr->adjs = v;
  glptr->depth = 0; //assuming inf to be 255
}


struct adjtree_leaf* construct_adjtree_leaf(struct adjtree_idx* p, uint32_t k, struct graphtree_leaf* v)
{
  struct adjtree_leaf* alptr = (struct adjtree_leaf*)malloc(sizeof(struct adjtree_leaf));
  alptr->parent = p;
  alptr->nodeid = k;
  alptr->adjptr = v;
}



void graphtree_inorder_traversal(struct graphtree_idx* root, void (*visitor)(uint32_t, struct adjtree_idx*)) //argument should be the value of the leaf node
{
  if(root->left_type != -1) //left child is either a inode or a value_node
    {
      if(root->left_type == 1 && root->leftchild.lchild) //its a value node
	{

	  visitor(root->leftchild.lchild->nodeid, root->leftchild.lchild->adjs);
	}
      else if(root->left_type == 0 && root->leftchild.ichild) //its an internal node
	{
	  graphtree_inorder_traversal(root->leftchild.ichild,visitor);
	}
      else
	{
	  assert(0); //something is wrong
	}
    }

  if(root->right_type != -1)
    {
      if(root->right_type == 1 && root->rightchild.lchild ) //its a value node
	{
	  visitor(root->rightchild.lchild->nodeid, root->rightchild.lchild->adjs);
	  // std::cout<<"Key="<<root->rightchild.vchild->key<<std::endl;
	}
      else if(root->right_type == 0 && root->rightchild.ichild)
	{
	  graphtree_inorder_traversal(root->rightchild.ichild,visitor);
	}
      else
	{
	  assert(0);
	}
    }
}


void adjtree_inorder_traversal(void* env, struct adjtree_idx* root, void (*visitor)(void* env, uint32_t, struct graphtree_leaf*)) //argument should be the value of the leaf node
{
  if(root->left_type != -1) //left child is either a inode or a value_node
    {
      if(root->left_type == 1 && root->leftchild.lchild) //its a value node
	{

	  visitor(env, root->leftchild.lchild->nodeid, root->leftchild.lchild->adjptr);
	}
      else if(root->left_type == 0 && root->leftchild.ichild) //its an internal node
	{
	  adjtree_inorder_traversal(env, root->leftchild.ichild,visitor);
	}
      else
	{
	  assert(0); //something is wrong
	}
    }

  if(root->right_type != -1)
    {
      if(root->right_type == 1 && root->rightchild.lchild ) //its a value node
	{
	  visitor(env, root->rightchild.lchild->nodeid, root->rightchild.lchild->adjptr);
	}
      else if(root->right_type == 0 && root->rightchild.ichild)
	{
	  adjtree_inorder_traversal(env, root->rightchild.ichild,visitor);
	}
      else
	{
	  assert(0);
	}
    }
}



uint8_t graphtree_insert_leafandValue(struct graphtree_idx* root, uint32_t new_key, struct adjtree_idx* new_value, struct graphtree_leaf* old_node, uint8_t level)
{
  assert(root->left_type  == -1 && root->right_type == -1);
  uint32_t side_new_key = key_hash_one_zero(new_key, level);
  uint32_t side_old_key = key_hash_one_zero(old_node->nodeid, level);
  if(side_old_key != side_new_key)
    {
      root->left_type = 1;
      root->right_type = 1;
      old_node->parent = root;
      struct graphtree_leaf* new_node = construct_graphtree_leaf(root, new_key, new_value);

      if(side_new_key == 0) /* new_key on left  and old_key on right */
	{
	  root->leftchild.lchild = new_node;
	  root->rightchild.lchild = old_node;
	  assert(root->left_type < 2);
	  return level;
	}
      else
	{
	  root->leftchild.lchild = old_node;
	  root->rightchild.lchild = new_node;
	  assert(root->left_type < 2);
	  return level;
	}
      assert(0);
      return level;
    }

  struct graphtree_idx* new_idx_node = construct_graphtree_idx();
  new_idx_node->parent = root;
  assert(level <255);

  if(side_new_key == 0) /* both belong to left side*/
    {
      root->left_type = 0;
      root->leftchild.ichild = new_idx_node; //overrides the vchild
      assert(root->left_type < 2);
    }
  else
    {
      root->right_type = 0;
      root->rightchild.ichild = new_idx_node;
    }
  uint32_t newlevel = graphtree_insert_leafandValue(new_idx_node, new_key, new_value, old_node, level+1);
  assert(root->left_type < 2);
  return newlevel;
}

//return type is void because many-changes are fe
uint8_t graphtree_insert_value(struct graphtree_idx* root, uint32_t new_key, struct adjtree_idx* new_value, uint8_t level)
{

  uint32_t side = key_hash_one_zero(new_key, level);

  if(side == 0)
    {

      if(root->left_type == 0)
	{
	if(root->leftchild.ichild)
	  {
	    assert(root->left_type < 2);
	    return graphtree_insert_value(root->leftchild.ichild, new_key, new_value, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      else if(root->left_type == 1)
	{

	  if(root->leftchild.lchild->nodeid == new_key)
	    {
	      assert(root->left_type < 2);
	      return level;
	    }

	  struct graphtree_idx* new_node = construct_graphtree_idx();
	  new_node->parent = root;
	  struct graphtree_leaf* old_leaf = root->leftchild.lchild;
	  assert(level <255);
	  int newlevel = graphtree_insert_leafandValue(new_node, new_key, new_value, old_leaf, level+1);

	  root->left_type = 0;
	  root->leftchild.ichild = new_node; //overrides the vchild
	  assert(root->left_type < 2);
	  return newlevel;
	  //return max(l1,l2);

	}

      else if(root->left_type == -1)
	{
	  struct graphtree_leaf* new_node = construct_graphtree_leaf(root, new_key, new_value);

	  root->left_type = 1;
	  root->leftchild.lchild = new_node;
	  return level;

	}
      else
	{
	  assert(0);
	}
      //////////////////////////////////////////////
      /// In summary                              //
      /// if root.left_type == 0 walk_down        //
      /// if root.left_type == 1                  //
      ///       create new_inode with             //
      ///       insert(new_inode, oldkey, level+1)//
      ///       insert(new_inode, newkey, level+1)//
      /// if root.left_type == -1                 //
      ///        creat new_vchild                 //
      //         add as root's leftchild         ///
      //////////////////////////////////////////////
    }


    if(side == 1)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->right_type == 0)
	{
	if(root->rightchild.ichild)
	  {
	    assert(root->left_type < 2);
	    return graphtree_insert_value(root->rightchild.ichild, new_key, new_value, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid right child        ///
      /////////////////////////////////////////////////
      else if(root->right_type == 1)
	{

	  //the value is alredy is already present
	  if(root->rightchild.lchild->nodeid == new_key)
	    {
	      assert(root->left_type < 2);
	      return level;
	    }

	  struct graphtree_idx* new_node = construct_graphtree_idx();
	  struct graphtree_leaf* old_leaf = root->rightchild.lchild;
	  new_node->parent = root;

	  uint8_t newlevel = graphtree_insert_leafandValue(new_node, new_key, new_value, root->rightchild.lchild, level+1);
	  ///////////////////////////////////////////////////
	  /// set new_node as root's right _child          ///
	  //////////////////////////////////////////////////
	  root->right_type = 0;
	  root->rightchild.ichild = new_node; //overrides the vchild
	  return newlevel;
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type == -1)
	{
	  struct graphtree_leaf* new_node = construct_graphtree_leaf(root, new_key, new_value);

	  //vnode* new_node = (vnode*) malloc( sizeof(vnode));
	  root->right_type = 1;
	  root->rightchild.lchild = new_node;
	  assert(root->left_type < 2);
	  return level;
	  //return new_node;
	}
      else
	{
	  assert(0);
	}
    }
    assert(0);
}


uint8_t adjtree_insert_value(struct adjtree_idx* root, uint32_t new_key, struct graphtree_leaf* new_value, uint8_t level)
{

  uint32_t side = key_hash_one_zero(new_key, level);

  if(side == 0)
    {
      if(root->left_type == 0)
	{
	if(root->leftchild.ichild)
	  {
	    return adjtree_insert_value(root->leftchild.ichild, new_key, new_value, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      else if(root->left_type == 1)
	{
	  if(root->leftchild.lchild->nodeid == new_key)
	    {
	      /*	      printf("debug:adjtree:key_already_present %d\n", new_key);*/
	      return level;
	    }

	  struct adjtree_idx* new_node = construct_adjtree_idx();
	  new_node->parent = root;

	  assert(level <255);
	  uint8_t l1 = adjtree_insert_value(new_node, new_key, new_value, level+1);
	  uint8_t l2= adjtree_insert_value(new_node, root->leftchild.lchild->nodeid, root->leftchild.lchild->adjptr, level+1);
	  free(root->leftchild.lchild);

	  root->left_type = 0;
	  root->leftchild.ichild = new_node; //overrides the vchild
	  return max(l1,l2);

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type == -1)
	{
	  struct adjtree_leaf* new_node = construct_adjtree_leaf(root, new_key, new_value);
	  // vnode* new_node = (vnode*) malloc( sizeof(vnode));
	  root->left_type = 1;
	  root->leftchild.lchild = new_node;
	  return level;
	  //return new_node;
	}
      else
	{
	  assert(0);
	}

    }


    if(side == 1)
    {

      if(root->right_type == 0)
	{
	if(root->rightchild.ichild)
	  {
	    return adjtree_insert_value(root->rightchild.ichild, new_key, new_value, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->right_type == 1)
	{

	  //the value is alredy is already present
	  if(root->rightchild.lchild->nodeid == new_key)
	    {
	      /*printf("debug:adjtree:key_already_present %d\n", new_key);*/
	      return level;
	    }

	  struct adjtree_idx* new_node = construct_adjtree_idx();
	  //inode* new_node = (inode*) malloc(sizeof(inode));

	  ////////////////////////////////////////////////////////////
	  /// Old: vchild --p--> root                             ///
	  /// Now: vchild --p--> new_node --p--> root            ///
	  //////////////////////////////////////////////////////////
	  new_node->parent = root;
	  //root->rightchild.vchild->parent = new_node;


	  uint8_t l1 = adjtree_insert_value(new_node, new_key, new_value, level+1);
	  uint8_t l2 = adjtree_insert_value(new_node, root->rightchild.lchild->nodeid, root->rightchild.lchild->adjptr, level+1);
	  free(root->rightchild.lchild);

	  ///////////////////////////////////////////////////
	  /// set new_node as root's left_child          ///
	  //////////////////////////////////////////////////
	  root->right_type = 0;
	  root->rightchild.ichild = new_node; //overrides the vchild
	  return max(l1,l2);
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type == -1)
	{
	  struct adjtree_leaf* new_node = construct_adjtree_leaf(root, new_key, new_value);


	  root->right_type = 1;
	  root->rightchild.lchild = new_node;
	  return level;
	  //return new_node;
	}
      else
	{
	  assert(0);
	}
    }
    assert(0);
}


void delete_graphtree_idx(struct graphtree_idx* root, struct graphtree_idx* in)
{
  if(in == root) return;
  if(!in->parent) assert(0);
  struct graphtree_idx* parent = in->parent;
  assert(in->left_type == -1);
  assert(in->right_type == -1);
  int side = -1;
  //find which side inode in is
  if(parent->left_type == 0)
    if(parent->leftchild.ichild == in)
      side = 0;
  if(parent->right_type == 0)
    if(parent->rightchild.ichild == in)
      {
	assert(side == -1);
	side = 1;
      }
  assert(side != -1);
  if(side  == 0)
    {
      parent->left_type = -1;
      parent->leftchild.ichild = 0;
      free(in);
    }
  if(side  == 1)
    {
      parent->right_type = -1;
      parent->rightchild.ichild = 0;
      free(in);
    }
  if(parent->left_type == -1 && parent->right_type == -1)
    delete_graphtree_idx(root, parent);
}

void delete_adjtree_idx(struct adjtree_idx* root, struct adjtree_idx* in)
{
  if(in == root) return;
  if(!in->parent) assert(0);
  struct adjtree_idx* parent = in->parent;
  assert(in->left_type == -1);
  assert(in->right_type == -1);
  int side = -1;
  //find which side inode in is
  if(parent->left_type == 0)
    if(parent->leftchild.ichild == in)
      side = 0;
  if(parent->right_type == 0)
    if(parent->rightchild.ichild == in)
      {
	assert(side == -1);
	side = 1;
      }
  assert(side != -1);
  if(side  == 0)
    {
      parent->left_type = -1;
      parent->leftchild.ichild = 0;
      free(in);
    }
  if(side  == 1)
    {
      parent->right_type = -1;
      parent->rightchild.ichild = 0;
      free(in);
    }
  if(parent->left_type == -1 && parent->right_type == -1)
    delete_adjtree_idx(root, parent);
}


struct adjtree_leaf* get_adjtree_leaf(struct adjtree_idx* tree)
{
  if(tree->left_type == 1)
    {
      assert(tree->leftchild.lchild);
      return tree->leftchild.lchild;
    }

  if(tree->right_type == 1)
    {
      assert(tree->rightchild.lchild);
      return tree->rightchild.lchild;
    }
  if(tree->left_type == 0)
    {
      struct adjtree_leaf* lchild = get_adjtree_leaf(tree->leftchild.ichild);
      assert(lchild);
      return lchild;

    }
  if(tree->right_type == 0)
    {
      struct adjtree_leaf* rchild = get_adjtree_leaf(tree->rightchild.ichild);
      assert(rchild);
      return rchild;
      //assert(0);
    }
  //assert(0);
  return 0;
}



void delete_adjtree_leaf(struct adjtree_idx* root, struct adjtree_leaf* vn)
{
  if(!vn->parent) assert(0);
  struct adjtree_idx* parent =vn->parent;
  int side = -1; //which side of parent is vn
  if(parent->left_type == 1)
    if(parent->leftchild.lchild == vn)
      side = 0;
  if(parent->right_type == 1)
    if(parent->rightchild.lchild == vn)
      {
	assert(side == -1);
	side = 1;
      }

  assert(side != -1);
  free(vn);
  if(side == 0)
    {
      //assert(parent->leftchild.vchild->key == vn->key);
      parent->left_type = -1;
      parent->leftchild.lchild = 0;
    }

  if(side == 1)
    {
      //assert(parent->rightchild.vchild->key == vn->key);
      parent->right_type = -1; //
      parent->rightchild.lchild  = 0;
    }
  if(parent->right_type == -1 && parent->left_type == -1)
    {
      if(parent != root)
      	delete_adjtree_idx(root, parent);
    }
}



void destruct_adjtree(struct adjtree_idx* root)
{
  struct adjtree_leaf* avnode = get_adjtree_leaf(root);
  while(avnode)
    {
      delete_adjtree_leaf(root,avnode);
      avnode= get_adjtree_leaf(root);
    }
  assert(root->left_type == -1);
  assert(root->right_type == -1);
}





//root
void delete_graphtree_leaf(struct graphtree_idx* root, struct graphtree_leaf* vn)
{
  if(!vn->parent) assert(0);
  struct graphtree_idx* parent =vn->parent;
  int side = -1; //which side of parent is vn
  if(parent->left_type == 1)
    if(parent->leftchild.lchild == vn)
      side = 0;
  if(parent->right_type == 1)
    if(parent->rightchild.lchild == vn)
      {
	assert(side == -1);
	side = 1;
      }

  assert(side != -1);
  if(vn->adjs){
    destruct_adjtree(vn->adjs);
    free(vn->adjs);
  }
  free(vn);

  if(side == 0)
    {
      //assert(parent->leftchild.vchild->key == vn->key);
      parent->left_type = -1;
      parent->leftchild.lchild = 0;

    }

  if(side == 1)
    {
      //assert(parent->rightchild.vchild->key == vn->key);
      parent->right_type = -1; //
      parent->rightchild.lchild  = 0;
    }
  if(parent->right_type == -1 && parent->left_type == -1)
    {
      if(parent != root)
      	delete_graphtree_idx(root, parent);
    }
}



int check_graphtree_nodeid(struct graphtree_idx* root, uint32_t key, uint8_t level)
{
  //should move exactly like insert
  uint32_t side = key_hash_one_zero(key,level);

  if(side == 0)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->left_type == 0)
	{
	if(root->leftchild.ichild)
	  {
	    return check_graphtree_nodeid(root->leftchild.ichild, key, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->left_type == 1)
	{
	  //we are at value-node (our item should be on left-side)
	  //not so fast..rambo...insertion may send items on other branch


	  if(root->leftchild.lchild->nodeid == key){
	    return 1;
	  }
	  return 0;

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type == -1)
	{
	  ///Oh..no...Scooby...we reached a dead-end
	  return 0;
	}
      else
	{
	  assert(0);
	}
      //////////////////////////////////////////////
      /// In summary                              //
      /// if root.left_type == 0 walk_down        //
      /// if root.left_type == 1                  //
      ///       check the given key               //
      ///                                         //
      ///                                         //
      /// if root.left_type == -1                 //
      ///           return false                  //
      //                                         ///
      //////////////////////////////////////////////
    }


    if(side == 1)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->right_type == 0)
	{
	if(root->rightchild.ichild)
	  {
	    return check_graphtree_nodeid(root->rightchild.ichild, key, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->right_type == 1)
	{
	  if(root->rightchild.lchild->nodeid == key) return 1;
	  return 0;
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type == -1)
	{
	  return 0;
	}
      else
	{
	  assert(0);
	}
    }
    assert(0);
}


int check_adjtree_nodeid(struct adjtree_idx* root, uint32_t key, uint8_t level)
{
  //should move exactly like insert
  uint32_t side = key_hash_one_zero(key,level);

  if(side == 0)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->left_type == 0)
	{
	if(root->leftchild.ichild)
	  {
	    return check_adjtree_nodeid(root->leftchild.ichild, key, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->left_type == 1)
	{
	  //we are at value-node (our item should be on left-side)
	  //not so fast..rambo...insertion may send items on other branch


	  if(root->leftchild.lchild->nodeid == key){
	    return 1;
	  }
	  return 0;

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type == -1)
	{
	  ///Oh..no...Scooby...we reached a dead-end
	  return 0;
	}
      else
	{
	  assert(0);
	}
      //////////////////////////////////////////////
      /// In summary                              //
      /// if root.left_type == 0 walk_down        //
      /// if root.left_type == 1                  //
      ///       check the given key               //
      ///                                         //
      ///                                         //
      /// if root.left_type == -1                 //
      ///           return false                  //
      //                                         ///
      //////////////////////////////////////////////
    }


    if(side == 1)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->right_type == 0)
	{
	if(root->rightchild.ichild)
	  {
	    return check_adjtree_nodeid(root->rightchild.ichild, key, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->right_type == 1)
	{
	  if(root->rightchild.lchild->nodeid == key) return 1;
	  return 0;
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type == -1)
	{
	  return 0;
	}
      else
	{
	  assert(0);
	}
    }
    assert(0);
}


struct graphtree_leaf* find_graphtree_leaf(struct graphtree_idx* root, uint32_t key, uint8_t level)
{
  //should move exactly like insert
  uint32_t side = key_hash_one_zero(key,level);

  if(side == 0)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->left_type == 0)
	{
	if(root->leftchild.ichild)
	  {
	    return find_graphtree_leaf(root->leftchild.ichild, key, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->left_type == 1)
	{
	  //we are at value-node (our item should be on left-side)
	  //not so fast..rambo...insertion may send items on other branch


	  if(root->leftchild.lchild->nodeid == key){
	    return root->leftchild.lchild;
	  }
	  return 0;

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type == -1)
	{
	  ///Oh..no...Scooby...we reached a dead-end
	  return 0;
	}
      else
	{
	  assert(0);
	}
      //////////////////////////////////////////////
      /// In summary                              //
      /// if root.left_type == 0 walk_down        //
      /// if root.left_type == 1                  //
      ///       check the given key               //
      ///                                         //
      ///                                         //
      /// if root.left_type == -1                 //
      ///           return false                  //
      //                                         ///
      //////////////////////////////////////////////
    }
    if(side == 1)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->right_type == 0)
	{
	if(root->rightchild.ichild)
	  {
	    return find_graphtree_leaf(root->rightchild.ichild, key, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->right_type == 1)
	{
	  if(root->rightchild.lchild->nodeid == key) return root->rightchild.lchild;
	  return 0;
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type == -1)
	{
	  return 0;
	}
      else
	{
	  assert(0);
	}
    }
    assert(0);
}


struct adjtree_leaf* find_adjtree_leaf(struct adjtree_idx* root, uint32_t key, uint8_t level)
{
  //should move exactly like insert
  uint32_t side = key_hash_one_zero(key,level);

  if(side == 0)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->left_type == 0)
	{
	if(root->leftchild.ichild)
	  {
	    return find_adjtree_leaf(root->leftchild.ichild, key, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->left_type == 1)
	{
	  //we are at value-node (our item should be on left-side)
	  //not so fast..rambo...insertion may send items on other branch


	  if(root->leftchild.lchild->nodeid == key){
	    return root->leftchild.lchild;
	  }
	  return 0;

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type == -1)
	{
	  ///Oh..no...Scooby...we reached a dead-end
	  return 0;
	}
      else
	{
	  assert(0);
	}
      //////////////////////////////////////////////
      /// In summary                              //
      /// if root.left_type == 0 walk_down        //
      /// if root.left_type == 1                  //
      ///       check the given key               //
      ///                                         //
      ///                                         //
      /// if root.left_type == -1                 //
      ///           return false                  //
      //                                         ///
      //////////////////////////////////////////////
    }
    if(side == 1)
    {
      ////////////////////////////////////////////////////////
      /// walk_down to leaf-inode                          ///
      /// leaf-inode : inode with left_type as 1 or -1    ///
      ///////////////////////////////////////////////////////
      if(root->right_type == 0)
	{
	if(root->rightchild.ichild)
	  {
	    return find_adjtree_leaf(root->rightchild.ichild, key, level+1);
	  }
	else
	  {
	    assert(0);
	  }
	}
      ///////////////////////////////////////////////////
      ///  at leaf-inode with valid leftchild        ///
      /////////////////////////////////////////////////
      else if(root->right_type == 1)
	{
	  if(root->rightchild.lchild->nodeid == key) return root->rightchild.lchild;
	  return 0;
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type == -1)
	{
	  return 0;
	}
      else
	{
	  assert(0);
	}
    }
    assert(0);
}



//////////////////////////////////////////
//Traverse the tree an  return a vnode //
////////////////////////////////////////

struct graphtree_leaf* get_graphtree_leaf(struct graphtree_idx* tree)
{
  if(tree->left_type == 1)
    {
      assert(tree->leftchild.lchild);
      return tree->leftchild.lchild;
    }

  if(tree->right_type == 1)
    {
      assert(tree->rightchild.lchild);
      return tree->rightchild.lchild;
    }
  if(tree->left_type == 0)
    {
      struct graphtree_leaf* lchild = get_graphtree_leaf(tree->leftchild.ichild);
      assert(lchild);
      return lchild;

    }
  if(tree->right_type == 0)
    {
      struct graphtree_leaf* rchild = get_graphtree_leaf(tree->rightchild.ichild);
      assert(rchild);
      return rchild;
      //assert(0);
    }
  //assert(0);
  return 0;
}





void destruct_graphtree(struct graphtree_idx* root)
{
  struct graphtree_leaf* avnode = get_graphtree_leaf(root);
  while(avnode)
    {
      delete_graphtree_leaf(root,avnode);
      avnode= get_graphtree_leaf(root);
    }

  assert(root->left_type == -1);
  assert(root->right_type == -1);
  free(root);
}





/*
  auxillary routines to help
  breadth-first-search operation

*/
int genZeroOne()
{
  long int a = mrand48();
  while(a == 0) a = mrand48();
  if(a < 0) return 0;
  return 1;
}


struct graphtree_leaf* pick_graphtree_leaf_rand(struct graphtree_idx* gptr)
{
  int side = genZeroOne();

  if(side == 1 && gptr->right_type == -1) side = 0; /* right-tree has dead-end */

  if(side == 0 && gptr->left_type == 1) /* try left side */
    return gptr->leftchild.lchild;


  if(side == 0 && gptr->left_type == 0) /* try left side */
      return pick_graphtree_leaf_rand(gptr->leftchild.ichild);


  if(gptr->right_type == 0)
    return pick_graphtree_leaf_rand(gptr->rightchild.ichild);

  if(gptr->right_type == 1)
    return gptr->rightchild.lchild;


  assert(0); /* both left and right ree are dead-ends */
  return 0;
}
