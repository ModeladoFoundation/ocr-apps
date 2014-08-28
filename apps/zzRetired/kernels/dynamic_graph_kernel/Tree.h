#include <iostream>
#include <cassert>
#include <stdlib.h>
#define value_type int

struct inode;
struct vnode  //value node store the key
{
  value_type key;
  inode* parent;
  vnode(inode* p, value_type k)
  {
    parent = p;
    key  = k;
  }
};

union child
{
  inode* ichild;
  vnode* vchild;
};


struct inode
{
  inode* parent;
  union child leftchild;
  union child rightchild;
  int left_type;
  int right_type; // to set the type of left and right 0 ==> inode 1==> vnode
  inode()
  {
    left_type = -1; //undef
    right_type = -1; //undef
    parent = 0;
  }
};


inode* init_tree()
{
  inode* t = new inode;
  return t;

}

void inorder_traversal(inode* root, void (*visitor)(int))
{
  if(root->left_type != -1) //left child is either a inode or a value_node
    {
      if(root->left_type == 1 && root->leftchild.vchild) //its a value node
	{
	  visitor(root->leftchild.vchild->key);
	  //std::cout<<"Key="<<root->leftchild.vchild->key<<std::endl;
	}
      else if(root->left_type == 0 && root->leftchild.ichild) //its an internal node
	{
	  inorder_traversal(root->leftchild.ichild,visitor);
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
	  visitor(root->rightchild.vchild->key);
	  // std::cout<<"Key="<<root->rightchild.vchild->key<<std::endl;
	}
      else if(root->right_type == 0 && root->rightchild.ichild)
	{
	  inorder_traversal(root->rightchild.ichild,visitor);
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

vnode* insert_value(inode* root, value_type new_value)
{
  //choose side (left==0 or right == 1)
  int side = getZeroOneRandomNumber();
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
	    return insert_value(root->leftchild.ichild, new_value);
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
	  inode* new_node = new inode;
	  ////////////////////////////////////////////////////////////
	  /// Old: vchild --p--> root                             ///
	  /// Now: vchild --p--> new_node --p--> root            ///
	  //////////////////////////////////////////////////////////
	  new_node->parent = root;
	  root->leftchild.vchild->parent = new_node;

	  /////////////////////////////////////////////////////////
	  ///  set new_node.leftchild as leftchild of root     ///
	  ///  set new_node.rightchild as new vchild(value)   ///
	  //////////////////////////////////////////////////////
	  new_node->left_type = 1;
	  new_node->right_type =1;
	  new_node->leftchild.vchild =  new vnode(new_node, new_value);
	  new_node->rightchild.vchild = root->leftchild.vchild;


	  ///////////////////////////////////////////////////
	  /// set new_node as root's left_child          ///
	  //////////////////////////////////////////////////
	  root->left_type = 0;
	  root->leftchild.ichild = new_node; //overrides the vchild
	  return new_node->leftchild.vchild;

	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->left_type = -1)
	{
	  vnode* new_node = new vnode(root, new_value);
	  root->left_type = 1;
	  root->leftchild.vchild = new_node;
	  return new_node;
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
      ///       new_inode.left = root.left_child  //
      ///       new_inode.right = new_vchild      //
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
	    return insert_value(root->rightchild.ichild, new_value);
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
	  inode* new_node = new inode;
	  ////////////////////////////////////////////////////////////
	  /// Old: vchild --p--> root                             ///
	  /// Now: vchild --p--> new_node --p--> root            ///
	  //////////////////////////////////////////////////////////
	  new_node->parent = root;
	  root->rightchild.vchild->parent = new_node;

	  /////////////////////////////////////////////////////////
	  ///  set new_node.leftchild as leftchild of root     ///
	  ///  set new_node.rightchild as new vchild(value)   ///
	  //////////////////////////////////////////////////////
	  new_node->left_type = 1;
	  new_node->right_type =1 ;
	  new_node->leftchild.vchild =  root->rightchild.vchild;
	  new_node->rightchild.vchild = new vnode(new_node, new_value);


	  ///////////////////////////////////////////////////
	  /// set new_node as root's left_child          ///
	  //////////////////////////////////////////////////
	  root->right_type = 0;
	  root->rightchild.ichild = new_node; //overrides the vchild
	  return new_node->rightchild.vchild;
	}
      ///////////////////////////////////////////////
      /// at leaf-inode with deadend            ////
      /////////////////////////////////////////////
      else if(root->right_type = -1)
	{
	  vnode* new_node = new vnode(root, new_value);
	  root->right_type = 1;
	  root->rightchild.vchild = new_node;
	  return new_node;
	}
      else
	{
	  assert(0);
	}


    }

    assert(0);

}

void delete_inode(inode* root, inode* in)
{
  if(in == root) return;
  if(!in->parent) assert(0);
  inode* parent = in->parent;
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
    delete_inode(root, parent);

}

//root
void delete_vnode(inode* root, vnode* vn)
{
  if(!vn->parent) assert(0);
  inode* parent =vn->parent;
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
      assert(parent->leftchild.vchild->key == vn->key);
      parent->left_type = -1;
      parent->leftchild.vchild = 0;
    }

  if(side == 1)
    {
      assert(parent->rightchild.vchild->key == vn->key);
      parent->right_type = -1; //
      parent->rightchild.vchild  = 0;
    }
  if(parent->right_type == -1 && parent->left_type == -1)
    {
      if(parent != root)
      	delete_inode(root, parent);
    }
}


////RMAT-JointWalk

void rmat_jointWalk(inode* root, void (*vis)(int, int))
{
  double a0 = .57;
  double b0 = .19;
  double c0 = .19;
  double d0 = .05;

  inode* srcGuide = root;
  inode* desGuide = root;
  vnode* srcV ;
  vnode* desV;
  bool flag = true;
  double p;
  while(flag)
    {

      p = drand48();

      if(p < a0)
	{
	  //Both move left if feasible
	  if(srcGuide->left_type == 1 && desGuide->left_type == 1)
	    {
	      srcV = srcGuide->leftchild.vchild;
	      desV = desGuide->leftchild.vchild;
	      flag = false;
	      break;
	    }
	  if(srcGuide->left_type == 0)
	    srcGuide = srcGuide->leftchild.ichild;
	  if(desGuide->left_type == 0)
	    desGuide = desGuide->leftchild.ichild;
	}
      else if(p >= a0  && p  < a0 + b0)
	{
	  //des moves right
	  //src moves left

	  if(srcGuide->left_type == 1 and desGuide->right_type == 1)
	    {
	      srcV = srcGuide->leftchild.vchild;
	      desV = desGuide->rightchild.vchild;
	      flag = false;
	      break;
	    }
	  if(srcGuide->left_type == 0)
	    srcGuide = srcGuide->leftchild.ichild;
	  if(desGuide->right_type == 0)
	    desGuide = desGuide->rightchild.ichild;
	}
      else if(p >= a0 + b0 && p < a0 + b0 + c0)
	{
	  //src moves right
	  //des moves left
	  if(srcGuide->right_type == 1 and desGuide->left_type == 1)
	    {
	      srcV = srcGuide->rightchild.vchild;
	      desV = desGuide->leftchild.vchild;
	      flag = false;
	      break;
	    }
	  if(srcGuide->right_type == 0)
	    srcGuide = srcGuide->rightchild.ichild;
	  if(desGuide->left_type == 0)
	    desGuide = desGuide->leftchild.ichild;
	}
  else
    {
      //src and des move right
      if(srcGuide->right_type == 1 and desGuide->right_type == 1)
	{
	  srcV = srcGuide->rightchild.vchild;
	  desV = desGuide->rightchild.vchild;
	  flag = false;
	  break;
	}
      if(srcGuide->right_type == 0)
	srcGuide = srcGuide->rightchild.ichild;
      if(desGuide->right_type == 0)
	desGuide = desGuide->rightchild.ichild;
    }

      ///////////Check for dead end////////////
      /// restart the walk
      if(srcGuide->left_type == -1 && srcGuide->right_type == -1)
	{

	  srcGuide = root;
	  desGuide = root;
	}
      if(desGuide->left_type == -1 && desGuide->right_type == -1)
	{
	  srcGuide = root;
	  desGuide = root;
	}

      //readjust pselection probablitites
      a0 *= 0.9 + 0.2 * drand48();
      b0 *= 0.9 + 0.2 * drand48();
      c0 *= 0.9 + 0.2 * drand48();
      d0 *= 0.9 + 0.2 * drand48();
      double S = a0 + b0 + c0 + d0;

      a0 /= S; b0 /= S; c0 /= S;
      // d /= S;
      // Ensure all values add up to 1, regardless of floating point errors
      d0 = 1. - a0 - b0 - c0;
    }
  vis(srcV->key, desV->key);
  //  std::cout<<"Rmat Walk "<<srcV->key<<" "<<desV->key<<std::endl;
}



vnode* choose_vnode(inode* tree, inode* root)
{
  //choose side (left==0 or right == 1)
  int side = getZeroOneRandomNumber();
  if(side == 0)
    {

      if(root->left_type == 0)
	{
	if(root->leftchild.ichild)
	  {
	    return choose_vnode(tree, root->leftchild.ichild);
	  }
	}
	else if(root->left_type == 1)
	  {
	    if(root->leftchild.vchild)
	      {
		return root->leftchild.vchild;
	      }
	  }
      return choose_vnode(tree, tree); //dead end ; restart
    }
    if(side == 1)
    {
      if(root->right_type == 0)
	{
	if(root->rightchild.ichild)
	  {
	    return choose_vnode(tree, root->rightchild.ichild);
	  }
	}
	else if(root->right_type == 1)
	  {
	    if(root->rightchild.vchild)
	      return root->rightchild.vchild;
	  }

      return choose_vnode(tree, tree); ///dead end
    }
    return 0;
}
