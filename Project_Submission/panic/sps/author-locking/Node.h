//
// Created by tylertownsend on 9/20/18.
//

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
  std::mutex lock;
  bool mark;
  bool sentinel;
  std::atomic<Node *> leftSnap;
  std::atomic<Node *> rightSnap;
  Node(int const& data);
  ~Node();
  int getData();
  void setData(int const& data);
  int getHeight();
  void setHeight(int const& height);
  Node *get(int field);
  Node* getLeft();
  Node* getRight();
  Node* getParent();
  void setLeft(Node* node);
  void setRight(Node* node);
  void setParent(Node* node);
};

#endif //CONCURRENTTREETRAVERALS_NODE_H
