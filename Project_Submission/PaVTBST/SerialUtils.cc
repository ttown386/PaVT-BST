//
// Created by ttown on 9/30/2018.
//

#include "../../include/SerialUtils.h"
#include "../../include/BinarySearchTree.h"
#include "../../include/Node.h"
#include <list>
#include <stack>

std::list<int> inOrderTraversal(BinarySearchTree &bst) {
  std::list<int> inOrderList;
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
      inOrderList.push_back(curr->getData());
      curr = curr->getRight();
    }
  }
  return inOrderList;
}
std::list<int> preOrderTraversal(BinarySearchTree &bst) {

  std::list<int> preOrderList;
  std::stack<Node*> stack;

  Node *curr = bst.getRoot();
  stack.push(curr);
  while (!stack.empty()) {

    curr=stack.top();
    preOrderList.push_back(curr->getData());
    stack.pop();

    if (curr->getRight()) {
      stack.push(curr->getRight());
    }
    if (curr->getLeft()) {
      stack.push(curr->getLeft());
    }
  }
  return std::list<int>();
}

