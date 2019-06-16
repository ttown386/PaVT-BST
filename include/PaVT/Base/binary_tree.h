#ifndef BINARY_TREE_H_
#define BINARY_TREE_H_

#include <mutex>
#include <atomic>

#include <PaVT/Base/node.h>
#include <PaVT/lock_manager.h>

namespace pavt {
// Constants
const int MAXBF = 1; // Max Balance Factor
const int MINBF = -1; // Min Balance Factor
const int iMin = std::numeric_limits<int>::min(); // Min Integer
const int iMax = std::numeric_limits<int>::max(); // Max Integer
const int LEFT = 0;
const int RIGHT = 1;
const int HERE = 2;

class Node : public base::Node {
 private:
  int key;
  Node *left;
  Node *right;
  Node *parent;
  int height;
 public:
  std::atomic<Node* > leftSnap;
  std::atomic<Node* > rightSnap;
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

namespace base {

class BinaryTree {
 protected:
  bool isAvl;
  pavt::Node* root;
  pavt::Node* maxSentinel;
  pavt::Node* minSentinel;
};
} // namespace base
} //namespace pavt
#endif // BINARY_TREE_H_