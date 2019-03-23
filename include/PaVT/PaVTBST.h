
//
// Created by ttown on 9/30/2018.
//

#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H

#include <mutex>
#include <atomic>

#include <PaVT/lock_manager.h>

class Node : public pavt::base::Node {
 private:
  int key;
  Node *left;
  Node *right;
  Node *parent;
  int height;
 public:
  std::atomic<Node *> leftSnap;
  std::atomic<Node *> rightSnap;
  Node(int const& key) {
    setKey(key);
    setLeft(nullptr);
    setRight(nullptr);
    setParent(nullptr);
    leftSnap = rightSnap = this;
    setHeight(0);
    mark = false;
  }
  ~Node() {
   if (getLeft()!=nullptr) delete getLeft();
   if (getRight()!=nullptr) delete getRight();
  }

  int getKey() {
    return key;
  }

  Node *get(int field) {
    if (field==0) return this->getLeft();
    if (field==1) return this->getRight();
    if (field==2) return this;
  }

  void setKey(int const &key) {
    this->key = key;
  }

  int getHeight() {
    return height;
  }

  void setHeight(int const &height) {
    this->height = height;
  }

  Node *getLeft() {
    return left;
  }

  Node *getRight() {
    return right;
  }

  Node *getParent() {
    return parent;
  }

  void setLeft(Node *node) {
    this->left = node;
  }

  void setRight(Node* node) {
    this->right = node;
  }

  void setParent(Node* node) {
    this->parent = node;
  }
};

class PaVTBST {
 protected:
  bool isAvl;
  Node *root;
  Node *maxSentinel;
  Node *minSentinel;
  int nextField(Node *node, int const &key);
  void rotateLeft(Node *child, Node *node, Node *parent);
  void rotateRight(Node *child, Node *node, Node *parent);
  int height(Node *node);
  void rebalance(Node *node);
  Node *traverse(Node *node, int const &key);
  
 public:
  PaVTBST(bool isAvl=false);
  ~PaVTBST();
  void insert(int const &key);
  void remove(int const &key);
  bool contains(int const &key);
  Node *getRoot();
  Node *getMinSentinel();
  Node *getMaxSentinel();
  static thread_local pavt::LockManager* lock_manager;
};

#endif //BINARYSEARCHTREE_H
