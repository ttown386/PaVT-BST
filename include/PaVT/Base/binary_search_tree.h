#ifndef BINARY_SEARCH_TREE_H_
#define BINARY_SEARCH_TREE_H_

#include <mutex>
#include <atomic>

#include <PaVT/Base/binary_tree.h>
#include <PaVT/Base/node.h>
#include <PaVT/lock_manager.h>

namespace pavt {
namespace base {

class BinarySearchTree : public BinaryTree {

 public:
  static thread_local pavt::LockManager* lock_manager;
  inline BinarySearchTree(bool isAvl=false) {
    this->isAvl = isAvl;

    minSentinel = new pavt::Node(iMin);
    maxSentinel = new pavt::Node(iMax);

    maxSentinel->setParent(minSentinel);
    minSentinel->setRight(maxSentinel);

    maxSentinel->leftSnap = minSentinel;
    minSentinel->rightSnap = maxSentinel;

    this->root = maxSentinel;
  }

  inline ~BinarySearchTree() {
    delete minSentinel;
  }

  virtual void insert(const int& key) = 0;
  virtual void remove(const int& key) = 0;
  bool contains(const int& key);
  pavt::Node *getRoot();
  pavt::Node *getMinSentinel();
  pavt::Node *getMaxSentinel();

 protected:
  pavt::Node* insert(pavt::Node* new_node);
  std::pair<pavt::Node*, pavt::Node*>* remove(pavt::Node* node, const int& key);
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