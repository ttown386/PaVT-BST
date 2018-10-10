/*!
 *
 */
//
// Created by tylertownsend on 9/20/18.
//

#ifndef CONCURRENTTREETRAVERALS_AVLTREE_H
#define CONCURRENTTREETRAVERALS_AVLTREE_H

#include "Node.h"


class AVLTree {
 private:
  Node* root;
  Node* insert(Node* node, int const& data);
  Node* remove(Node* node, int const& data);
  bool contains(Node* node, int const& data);
  Node* rotateLeft(Node* node);
  Node* rotateRight(Node* node);
  int findMax(Node* node);
  int height(Node* node);
  int balanceFactor(Node* node);
  Node* rebalance(Node* node);

 public:
  AVLTree();
  ~AVLTree();
  bool insert(int const& data);
  void remove(int const& data);
  bool contains(int const& data);

};


#endif //CONCURRENTTREETRAVERALS_AVLTREE_H
