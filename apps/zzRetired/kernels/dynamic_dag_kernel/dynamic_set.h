//my own response to avl,treap etc.
//for emphermal sets (sets that have very short life)
//the idea is to keep operations really simple
//at the cost of sightly unbalanced-tree
//left-right rotation is avoided by hashing the key to left-or-right child
//mixing of  hash and binary-tree is necessary --
//since we can never estimate  size of input-elements
//its both an hash and a set (haset or sash)



//the code is modification of Tree.h
//That generates dynamic social network

#include <stdint.h>
#include "assert.h"
//#include <iostream>
//#include <cassert>
#include <stdlib.h>



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
#define key_type uint32_t
#define value_type void*

extern uint32_t hashlittle( const void *key, size_t length, uint32_t initval);

uint8_t max(uint8_t l1, uint8_t l2)
{
  if(l1 > l2) return l1;
  return l2;
}
struct inode;
struct vnode  //value node store the key
{
  key_type key;
  value_type value;

  struct inode* parent;


};

struct vnode* construct_vnode(struct inode* p, key_type k, value_type v)
{
  struct vnode* vn = (struct vnode*) malloc(sizeof(struct vnode));
  vn->parent = p;
  vn->key = k;
  vn->value = v;
  return vn;
}


union child
{
  struct inode* ichild;
  struct vnode* vchild;
};


struct inode
{
  struct inode* parent;
  union child leftchild;
  union child rightchild;
  int left_type;
  int right_type; // to set the type of left and right 0 ==> inode 1==> vnode
};


void copy_inode(struct inode* copy_to, struct inode* const copy_from)
{
  copy_to->parent = copy_from->parent;
  copy_to->leftchild = copy_from->leftchild;
  copy_to->rightchild = copy_from->rightchild;
  copy_to->left_type = copy_from->left_type;
  copy_to->right_type = copy_from->right_type;
  return;

}

struct inode* construct_inode()
{
  struct inode* in = (struct inode*) malloc(sizeof(struct inode));
  in->left_type = -1; //undef
  in->right_type = -1; //undef
  in->parent = 0;
  return in;
}

void inorder_traversal(void* env, struct inode* root, void (*visitor)(void* env, key_type, value_type))
{

  if(root->left_type != -1) //left child is either a inode or a value_node
    {
      if(root->left_type == 1 && root->leftchild.vchild) //its a value node
	{

	  visitor(env, root->leftchild.vchild->key, root->leftchild.vchild->value);
	  //std::cout<<"Key="<<root->leftchild.vchild->key<<std::endl;
	}
      else if(root->left_type == 0 && root->leftchild.ichild) //its an internal node
	{
	  inorder_traversal(env, root->leftchild.ichild,visitor);
	}
      else
	{
	  assert(0); //something is wrong
	}
    }

  if(root->right_type != -1)
    {
      if(root->right_type == 1 && root->rightchild.vchild ) //its a value node
	{
	  visitor(env, root->rightchild.vchild->key, root->rightchild.vchild->value);
	  // std::cout<<"Key="<<root->rightchild.vchild->key<<std::endl;
	}
      else if(root->right_type == 0 && root->rightchild.ichild)
	{
	  inorder_traversal(env,  root->rightchild.ichild,visitor);
	}
      else
	{
	  assert(0);
	}

    }

}

int getZeroOneRandomNumber()
{
  long int randnum;
  do
    {
      randnum = mrand48();
    }
  while(randnum == 0);

  int  zero_one =1;
  if(randnum < 0) zero_one = 0;

  return zero_one;
}

uint32_t one = 1;
uint32_t key_hash_one_zero(key_type new_value, uint8_t level)
{
  uint32_t h = level;

  h = hashlittle(&new_value, 4, h);
  h = h & one;
  return h;


}

//return type is void because many-changes are fe
uint8_t insert_value(struct inode* root, key_type new_key, value_type new_value, uint8_t level)
{
  //choose side (left==0 or right == 1)
  //  int side = getZeroOneRandomNumber();
  //hash new_value to  either left-size of right-side
  //key_type is fixed uint32_t
  //TODO: add level to the mix
  uint32_t side = key_hash_one_zero(new_key, level);
  //std::cout<<new_value<<" "<<level<<" --> side "<<std::endl;
  //printf("hash-val= %d %d %d \n", new_value, level, side);
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
	    return insert_value(root->leftchild.ichild, new_key, new_value,level+1);
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
	  //inode* new_node = (inode*) malloc(sizeof(inode));
	  //if the key is already present
	  if(root->leftchild.vchild->key == new_key)
	    return level;

	  struct inode* new_node = construct_inode();
	  ////////////////////////////////////////////////////////////
	  /// Old: vchild --p--> root                             ///
	  /// Now: vchild --p--> new_node --p--> root            ///
	  //////////////////////////////////////////////////////////
	  new_node->parent = root;
	  ////not sure about this; we have have to let go of vchild
	  ////give it back to the pool
	  //root->leftchild.vchild->parent = new_node;



	  /////////////////////////////////////////////////////////
	  ///  insert oldkey at one level down                 ///
	  ///  insert newkey at one level down                ///
	  //////////////////////////////////////////////////////


	  // new_node->left_type = 1;
	  //new_node->right_type =1;
	  //new_node->leftchild.vchild =  new vnode(new_node, new_value);
	  //new_node->rightchild.vchild = root->leftchild.vchild;
	  assert(level <255);
	  uint8_t l1 = insert_value(new_node, new_key, new_value, level+1);
	  uint8_t l2= insert_value(new_node, root->leftchild.vchild->key,root->leftchild.vchild->value, level+1);
	  free(root->leftchild.vchild);
	  ///////////////////////////////////////////////////
	  /// set new_node as root's left_child          ///
	  //////////////////////////////////////////////////
	  root->left_type = 0;
	  root->leftchild.ichild = new_node; //overrides the vchild
	  return max(l1,l2);

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type = -1)
	{
	  struct vnode* new_node = construct_vnode(root, new_key, new_value);
	  // vnode* new_node = (vnode*) malloc( sizeof(vnode));
	  root->left_type = 1;
	  root->leftchild.vchild = new_node;
	  return level;
	  //return new_node;
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
	    return insert_value(root->rightchild.ichild, new_key, new_value, level+1);
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
	  if(root->rightchild.vchild->key == new_key)
	    return level;

	  struct inode* new_node = construct_inode();
	  //inode* new_node = (inode*) malloc(sizeof(inode));

	  ////////////////////////////////////////////////////////////
	  /// Old: vchild --p--> root                             ///
	  /// Now: vchild --p--> new_node --p--> root            ///
	  //////////////////////////////////////////////////////////
	  new_node->parent = root;
	  //root->rightchild.vchild->parent = new_node;


	  uint8_t l1 = insert_value(new_node, new_key, new_value, level+1);
	  uint8_t l2 = insert_value(new_node, root->rightchild.vchild->key,root->rightchild.vchild->value, level+1);
	  free(root->rightchild.vchild);

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
      else if(root->right_type = -1)
	{
	  struct vnode* new_node = construct_vnode(root, new_key, new_value);
	  //vnode* new_node = (vnode*) malloc( sizeof(vnode));
	  root->right_type = 1;
	  root->rightchild.vchild = new_node;
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

void delete_inode(struct inode* root, struct inode* in)
{
  if(in == root) return;
  if(!in->parent) assert(0);
  struct inode* parent = in->parent;
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
    delete_inode(root, parent);

}

//root
void delete_vnode(struct inode* root, struct vnode* vn)
{
  if(!vn->parent) assert(0);
  struct inode* parent =vn->parent;
  int side = -1; //which side of parent is vn
  if(parent->left_type == 1)
    if(parent->leftchild.vchild == vn)
      side = 0;
  if(parent->right_type == 1)
    if(parent->rightchild.vchild == vn)
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
      parent->leftchild.vchild = 0;

    }

  if(side == 1)
    {
      //assert(parent->rightchild.vchild->key == vn->key);
      parent->right_type = -1; //
      parent->rightchild.vchild  = 0;
    }
  if(parent->right_type == -1 && parent->left_type == -1)
    {
      if(parent != root)
      	delete_inode(root, parent);
    }
}




value_type search_value(struct inode* root, key_type key, uint8_t level)
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
	    return search_value(root->leftchild.ichild, key, level+1);
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


	  if(root->leftchild.vchild->key == key){
	    return root->leftchild.vchild->value;
	  }
	  return 0;

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type = -1)
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
	    return search_value(root->rightchild.ichild, key, level+1);
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
	  if(root->rightchild.vchild->key == key) return root->rightchild.vchild->value;
	  return 0;
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type = -1)
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





struct vnode* search_vnode(struct inode* root, key_type key, uint8_t level)
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
	    return search_vnode(root->leftchild.ichild, key, level+1);
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


	  if(root->leftchild.vchild->key == key){
	    return root->leftchild.vchild;
	  }
	  return 0;

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type = -1)
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
	    return search_vnode(root->rightchild.ichild, key, level+1);
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
	  if(root->rightchild.vchild->key == key) return root->rightchild.vchild;
	  return 0;
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type = -1)
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

/////////////////////////////////////////
// same as  to delete_inode --but returns      //
//                                     //
//                                     //
//                                     //
/////////////////////////////////////////
struct inode* destruct_inode(struct inode* root, struct inode* in)
{
  if(in == root) return 0;
  if(!in->parent) assert(0);
  struct inode* parent = in->parent;
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
  if(side  == -1)
    {
      parent->right_type = -1;
      parent->rightchild.ichild = 0;
      free(in);
    }
  if(parent->left_type == -1 && parent->right_type == -1)
    return destruct_inode(root, parent);
}

//root
struct inode* destruct_vnode(struct inode* root, struct vnode* vn)
{
  if(!vn->parent) assert(0);
  struct inode* parent =vn->parent;
  int side = -1; //which side of parent is vn
  if(parent->left_type == 1)
    if(parent->leftchild.vchild == vn)
      side = 0;
  if(parent->right_type == 1)
    if(parent->rightchild.vchild == vn)
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
      parent->leftchild.vchild = 0;
    }

  if(side == 1)
    {
      //assert(parent->rightchild.vchild->key == vn->key);
      parent->right_type = -1; //
      parent->rightchild.vchild  = 0;
    }
  if(parent->right_type == -1 && parent->left_type == -1)
    {
      if(parent != root)
      	return destruct_inode(root, parent);
    }
  return parent;
}

//////////////////////////////////////////
//Traverse the tree an  return a vnode //
////////////////////////////////////////

struct vnode* get_vnode(struct inode* tree)
{
  if(tree->left_type == 1)
    {
      assert(tree->leftchild.vchild);
      return tree->leftchild.vchild;
    }

  if(tree->right_type == 1)
    {
      assert(tree->rightchild.vchild);
      return tree->rightchild.vchild;
    }
  if(tree->left_type == 0)
    {
      struct vnode* lchild = get_vnode(tree->leftchild.ichild);
      assert(lchild);
      return lchild;

    }
  if(tree->right_type == 0)
    {
      struct vnode* rchild = get_vnode(tree->rightchild.ichild);
      assert(rchild);
      return rchild;
      //assert(0);
    }
  //assert(0);
  return 0;
}


void destruct_tree(struct inode* root)
{
  struct vnode* avnode = get_vnode(root);
  while(avnode)
    {
      delete_vnode(root,avnode);
      avnode= get_vnode(root);
    }
  assert(root->left_type == -1);
  assert(root->right_type == -1);


}

int is_empty(struct inode* root)
{
  if(root->left_type == -1 && root->right_type == -1)
    return 1;
  return 0;
}
