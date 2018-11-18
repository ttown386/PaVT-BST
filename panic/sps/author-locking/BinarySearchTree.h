
//
// Created by ttown on 9/30/2018.
//

#ifndef CONCURRENTTREETRAVERALS_BINARYSEARCHTREE_H
#define CONCURRENTTREETRAVERALS_BINARYSEARCHTREE_H

#include "Node.h"

class BinarySearchTree {
 public:
// private:
  bool isAvl;
  Node *root;
  Node *maxSentinel;
  Node *minSentinel;
  void rebalanceSynchronized(Node *node);
  void rotate(Node *child, Node *node, Node *parent, bool left);
  Node *traverse(Node *node, int const &data);
  void updateSnaps(Node *node);
  void updateSnaps(Node *start, Node *toUpdate, Node *parent);
  void applyRemove(Node *rightNode, Node *node, Node *parent, Node *left, Node *right, bool leftChild, Node *succ, Node *succParent, Node *succRight, Node *succRightSnapshot);

// public:
  BinarySearchTree(bool isAvl=false);
  ~BinarySearchTree();
  void insert(int const& data);
  void remove(int const& data, int &id);
  bool contains(int const& data);
  Node *getRoot();

};

#endif //CONCURRENTTREETRAVERALS_BINARYSEARCHTREE_H
