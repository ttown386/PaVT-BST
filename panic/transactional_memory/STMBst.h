
//
// Created by ttown on 9/30/2018.
//

#ifndef CONCURRENTTREETRAVERALS_STMBST_H
#define CONCURRENTTREETRAVERALS_STMBST_H

#include "Node.h"

class STMBst {
 public:
// private:
  bool isAvl;
  Node *root=nullptr;
  Node *maxSentinel;
  Node *minSentinel;
  int nextField(Node *node, int const &data);
  void updateHeights(Node *curr);
  Node *rotateLeft(Node *node);
  Node *rotateRight(Node *node);
  int height(Node *node);
  int balanceFactor(Node *node);
  void rebalance(Node *node);
  Node *traverse(Node *node, int const &data);
  void updateSnaps(Node *node);
  void updateSnaps(Node *start, Node *toUpdate, Node *parent);

// public:
	STMBst(bool isAvl=false);
  ~STMBst();
  void insert(int const& data);
  void remove(int const& data, int &id);
  bool contains(int const& data);
  Node *getRoot();

};

#endif //CONCURRENTTREETRAVERALS_BINARYSEARCHTREE_H
