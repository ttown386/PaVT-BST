#ifndef UTITLITIESBST_H
#define UTITLITIESBST_H

#include <list>
#include <stack>
#include <queue>
#include <iostream>
#include "PaVTBST.h"

class NodeDepth {
 public:
  Node * node;
  int depth;
  NodeDepth(Node *n, int d) {
    node=n;
    depth = d;
  }
};

void printInOrderTraversal(PaVTBST &bst) {

  std::stack<Node*> stack;

  // TODO: bst should be a constant reference so as to not alter input
  Node *curr = bst.getRoot();

  while (!stack.empty() || curr!=nullptr) {

    if (curr!=nullptr) {
      stack.push(curr);
      curr = curr->getLeft();
    } else {
      curr = stack.top();
      stack.pop();
      std::cout<<(curr->getKey())<<" ";
      curr = curr->getRight();
    }
  }
  std::cout<<std::endl;
}

void printPreOrderTraversal(PaVTBST &bst) {

  std::stack<Node*> stack;

  Node *curr = bst.getRoot();
  stack.push(curr);

  while (!stack.empty()) {

    curr=stack.top();
    std::cout<<(curr->getKey())<<" ";
    stack.pop();

    if (curr->getRight()) {
      stack.push(curr->getRight());
    }
    if (curr->getLeft()) {
      stack.push(curr->getLeft());
    }
  }
  std::cout<<std::endl;
}

bool check (PaVTBST &bst) {
  Node *curr = bst.getMinSentinel();
  int currVal = curr->getKey();
  Node *last = bst.getMaxSentinel();
  while (curr!=last) {
    curr = curr->rightSnap;
    int nextVal = curr->getKey();
    if (nextVal <= currVal) return false;
    currVal = nextVal;
  }
  return true;
}

void printTreeDepth(PaVTBST &bst) {

  Node *start = bst.getRoot();
  std::queue<NodeDepth *> q;
  q.push(new NodeDepth(start, 0));

  int prevDepth = 0;
  while(!q.empty()) {

    NodeDepth *curr = q.front();
    Node *currNode = curr->node;
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
      q.push(new NodeDepth(currNode->getLeft(), currDepth + 1));
      q.push(new NodeDepth(currNode->getRight(), currDepth + 1));
    } 
  }
}

#endif //UTITLITIESBST_H