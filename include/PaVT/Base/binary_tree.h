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
namespace base {

class BinaryTree {
 public:
  inline BinaryTree(bool isAvl=false) {
    this->isAvl = isAvl;

    minSentinel = new pavt::Node(iMin);
    maxSentinel = new pavt::Node(iMax);

    maxSentinel->setParent(minSentinel);
    minSentinel->setRight(maxSentinel);

    maxSentinel->leftSnap = minSentinel;
    minSentinel->rightSnap = maxSentinel;

    this->root = maxSentinel;
  }

  inline ~BinaryTree() {
    delete minSentinel;
  }

  bool contains(const int& key);
  pavt::Node *getRoot();
  pavt::Node *getMinSentinel();
  pavt::Node *getMaxSentinel();
  static thread_local pavt::LockManager* lock_manager;

  virtual void insert(const int& key) = 0;
  virtual void remove(const int& key) = 0;

 protected:
  bool isAvl;
  pavt::Node *root;
  pavt::Node *maxSentinel;
  pavt::Node *minSentinel;

  int nextField(pavt::Node *node, const int &key);
  pavt::Node* traverse(pavt::Node *node, const int &key);
  void lock(Node* node);
  bool tryLock(Node* node);
  void unlock();
  void unlockAll();
};
} // namespace base
} //namespace pavt
#endif // BINARY_TREE_H_