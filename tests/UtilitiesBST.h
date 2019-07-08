#ifndef UTITLITIESBST_H
#define UTITLITIESBST_H

#include <list>
#include <vector>
#include <stack>
#include <queue>
#include <iostream>

#include <PaVT/Base/binary_tree.h>
#include <PaVT/bst.h>

using namespace pavt;
using namespace base;

class NodeDepth {
 public:
  BinaryTree::Node * node;
  int depth;
  NodeDepth(BinaryTree::Node *n, int d) {
    node=n;
    depth = d;
  }
};

inline 
std::vector<int> inOrderTraversal(BST &bst) {
  std::stack<BinaryTree::Node*> stack;

  std::vector<int> return_vals;
  BinaryTree::Node *curr = bst.getRoot();

  while (!stack.empty() || curr!=nullptr) {

    if (curr!=nullptr) {
      stack.push(curr);
      curr = curr->left;
    } else {
      curr = stack.top();
      stack.pop();
      return_vals.push_back(curr->getKey());
      curr = curr->right;
    }
  }
  return return_vals;
}

inline 
std::vector<int> preOrderTraversal(BST &bst) {

  std::stack<BinaryTree::Node*> stack;
  std::vector<int> return_vals;
  BinaryTree::Node *curr = bst.getRoot();
  stack.push(curr);

  while (!stack.empty()) {

    curr=stack.top();
    return_vals.push_back(curr->getKey());
    stack.pop();

    if (curr->right) {
      stack.push(curr->right);
    }
    if (curr->left) {
      stack.push(curr->left);
    }
  }
  return return_vals;
}

inline
void printInOrderTraversal(BST &bst) {

  std::stack<BinaryTree::Node*> stack;

  // TODO: bst should be a constant reference so as to not alter input
  BinaryTree::Node *curr = bst.getRoot();

  while (!stack.empty() || curr!=nullptr) {

    if (curr!=nullptr) {
      stack.push(curr);
      curr = curr->left;
    } else {
      curr = stack.top();
      stack.pop();
      std::cout<<(curr->getKey())<<" ";
      curr = curr->right;
    }
  }
  std::cout<<std::endl;
}

inline
void printPreOrderTraversal(BST &bst) {

  std::stack<BinaryTree::Node*> stack;

  BinaryTree::Node *curr = bst.getRoot();
  stack.push(curr);

  while (!stack.empty()) {

    curr=stack.top();
    std::cout<<(curr->getKey())<<" ";
    stack.pop();

    if (curr->right) {
      stack.push(curr->right);
    }
    if (curr->left) {
      stack.push(curr->left);
    }
  }
  std::cout<<std::endl;
}

inline
bool check (BST &bst) {
  BinaryTree::Node *curr = bst.getMinSentinel();
  int currVal = curr->getKey();
  BinaryTree::Node *last = bst.getMaxSentinel();
  while (curr!=last) {
    curr = ((BinarySearchTree::Node*)curr)->rightSnap;
    int nextVal = curr->getKey();
    if (nextVal <= currVal) return false;
    currVal = nextVal;
  }
  return true;
}

inline
void printTreeDepth(BST &bst) {

  BinaryTree::Node *start = bst.getRoot();
  std::queue<NodeDepth *> q;
  q.push(new NodeDepth(start, 0));

  int prevDepth = 0;
  while(!q.empty()) {

    NodeDepth *curr = q.front();
    BinaryTree::Node *currNode = curr->node;
    int currDepth = curr->depth;
    q.pop();
    delete curr;

    if (currDepth!=prevDepth) {
      std::cout<<"\n";
      prevDepth = currDepth;
    }
    if (currNode!=nullptr) {
      std::cout<<currNode->getKey()<<" ";
    } else {
      std::cout<<"- ";
    }
    if (currNode!=nullptr) {
      q.push(new NodeDepth(currNode->left, currDepth + 1));
      q.push(new NodeDepth(currNode->right, currDepth + 1));
    } 
  }
}

#endif //UTITLITIESBST_H