//
// Created by tylertownsend on 9/20/18.
//
// Modified by Robert Bland for transactional memory
// Have to get rid of locks and atomic nodes

#ifndef CONCURRENTTREETRAVERALS_NODE_H
#define CONCURRENTTREETRAVERALS_NODE_H

#include <mutex>
#include <atomic>

class Node {
 private:
  int data;
  Node* left;
  Node* right;
  Node* parent;
  int height;
 public:
  bool mark;
  bool sentinel;
  Node* leftSnap;
  Node* rightSnap;
  Node(int const& data) transaction_safe;
  ~Node();
  int getData() transaction_safe;
  void setData(int const& data) transaction_safe;
  int getHeight() transaction_safe;
  void setHeight(int const& height) transaction_safe;
  Node *get(int field) transaction_safe;
  Node* getLeft() transaction_safe;
  Node* getRight() transaction_safe;
  Node* getParent() transaction_safe;
  void setLeft(Node* node) transaction_safe;
  void setRight(Node* node) transaction_safe;
  void setParent(Node* node) transaction_safe;
};

#endif //CONCURRENTTREETRAVERALS_NODE_H
