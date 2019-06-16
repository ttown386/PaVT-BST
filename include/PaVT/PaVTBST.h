//
// Created by ttown on 9/30/2018.
//

#ifndef PAVT_BINARY_SEARCH_TREE_H_
#define PAVT_BINARY_SEARCH_TREE_H_ 

#include <mutex>
#include <atomic>

#include <PaVT/Base/binary_tree.h>
#include <PaVT/Base/binary_search_tree.h>
#include <PaVT/lock_manager.h>

namespace pavt {

class PaVTBST : public base::BinarySearchTree {
 public:
  using base::BinarySearchTree::BinarySearchTree;
  void insert(const int &key);
  void remove(const int &key);

 protected:
  void rotateLeft(Node *child, Node *node, Node *parent);
  void rotateRight(Node *child, Node *node, Node *parent);
  int height(Node *node);
  void rebalance(Node *node);
};
}
#endif // PAVT_BINARY_SEARCH_TREE_H_
