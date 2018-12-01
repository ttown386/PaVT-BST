
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
  void rotateLeft(Node *child, Node *node, Node *parent);
  void rotateRight(Node *child, Node *node, Node *parent);
  int height(Node *node);
  void rebalance(Node *node);
  Node *traverse(Node *node, int const &data);

// public:
	STMBst(bool isAvl=false);
  ~STMBst();
  void insert(int const& data);
  void remove(int const& data);
  bool contains(int const& data);
  Node *getRoot();

};

#endif //CONCURRENTTREETRAVERALS_STMBST_H
