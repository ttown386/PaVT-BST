//
// Created by tylertownsend on 9/20/18.
//

#ifndef CONCURRENTTREETRAVERALS_NODE_H
#define CONCURRENTTREETRAVERALS_NODE_H

#include <mutex>

class Node {
 private:
  int data;
  Node* left;
  Node* right;
  Node* parent;
  int height;
 public:
  std::mutex lock;
  bool mark;
  bool sentinel;
  Node *pred;
  Node *succ;
  Node(int const& data);
  ~Node();
  int getData();
  void setData(int const& data);
  int getHeight();
  void setHeight(int const& height);
  Node* getLeft();
  Node* getRight();
  Node* getParent();
  void setLeft(Node* node);
  void setRight(Node* node);
  void setParent(Node* node);
};

#endif //CONCURRENTTREETRAVERALS_NODE_H
