//
// Created by tylertownsend on 9/20/18.
//

#ifndef CONCURRENTTREETRAVERALS_NODE_H
#define CONCURRENTTREETRAVERALS_NODE_H

class Node {
 private:
  int data;
  Node* left;
  Node* right;
 public:
  Node(int const& data);
  ~Node();
  int getData();
  void setData(int const& data);
  Node* getLeft();
  Node* getRight();
  void setLeft(Node* node);
  void setRight(Node* node);
};

#endif //CONCURRENTTREETRAVERALS_NODE_H
