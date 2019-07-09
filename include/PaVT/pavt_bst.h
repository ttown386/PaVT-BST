#ifndef TTREE_INCLUDE_PAVT_PAVT_BST_H_ 
#define TTREE_INCLUDE_PAVT_PAVT_BST_H_

#include <mutex>
#include <atomic>

#include <PaVT/Base/binary_tree.h>
#include <PaVT/Base/locknode.h>
#include <PaVT/lock_manager.h>

namespace pavt {

class PaVTBST : public base::BinaryTree {
 public:
  class Node : public base::BinaryTree::Node, public base::LockNode{
   public:
    std::atomic<Node* > leftSnap;
    std::atomic<Node* > rightSnap;
    Node(const int& key) : BinaryTree::Node(key) {
      leftSnap = rightSnap = this;
      _mark = false;
    }
    ~Node() {}
    inline void Mark() { _mark = true; }
    inline bool IsMarked() { return _mark; }
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
  friend bool ValidatePaVTBST(PaVTBST& bst);
};
} //namespace pavt
#endif // TTREE_INCLUDE_PAVT_PAVT_BST_H_
