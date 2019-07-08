#ifndef BINARY_SEARCH_TREE_H_
#define BINARY_SEARCH_TREE_H_

#include <mutex>
#include <atomic>

#include <PaVT/Base/binary_tree.h>
#include <PaVT/Base/locknode.h>
#include <PaVT/lock_manager.h>

namespace pavt {

class BinarySearchTree : public base::BinaryTree {
 public:
  class Node : public base::BinaryTree::Node, public base::LockNode{
   public:
    std::atomic<Node* > leftSnap;
    std::atomic<Node* > rightSnap;
    Node(const int& key) : BinaryTree::Node(key) {
      leftSnap = rightSnap = this;
      mark = false;
    }
    ~Node() {}
  };
  static thread_local pavt::LockManager* lock_manager;
  virtual void insert(const int& key) = 0;
  virtual void remove(const int& key) = 0;
  bool contains(const int& key);
  BinaryTree::Node *getRoot();
  Node *getMinSentinel();
  Node *getMaxSentinel();

 protected:
  Node* maxSentinel;
  Node* minSentinel;
  Node* insert(Node* new_node);
  std::pair<Node*, Node*>* remove(Node* node, const int& key);
  int nextField(BinaryTree::Node *node, const int &key);
  Node* traverse(Node *node, const int &key);
  void lock(Node* node);
  bool tryLock(Node* node);
  void unlock();
  void unlockAll();
};
} //namespace pavt
#endif // BINARY_TREE_H_