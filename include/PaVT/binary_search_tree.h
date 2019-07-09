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
  virtual void Insert(const int& key) = 0;
  virtual void Remove(const int& key) = 0;
  virtual bool Contains(const int& key) = 0;

 protected:
  Node* maxSentinel;
  Node* minSentinel;
  Node* Insert(Node* new_node);
  std::pair<Node*, Node*>* Remove(Node* node, const int& key);
  bool Contains(Node* start_node, const int& key);
  int NextField(BinaryTree::Node *node, const int &key);
  Node* Traverse(Node *node, const int &key);
  friend bool ValidatePaVTBST(BinarySearchTree& bst);
};
} //namespace pavt
#endif // BINARY_TREE_H_