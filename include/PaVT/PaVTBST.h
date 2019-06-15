//
// Created by ttown on 9/30/2018.
//

#ifndef PAVT_BINARY_SEARCH_TREE_H_
#define PAVT_BINARY_SEARCH_TREE_H_ 

#include <mutex>
#include <atomic>

#include <PaVT/Base/binary_tree.h>
#include <PaVT/lock_manager.h>

namespace pavt {

class PaVTBST : public base::BinaryTree {
 public:
  using base::BinaryTree::BinaryTree;
  void insert(const int &key);
  void remove(const int &key);

 protected:
  Node* insert(Node* node);
  std::pair<Node*, Node*>* remove(Node* node, const int& key);

  void rotateLeft(Node *child, Node *node, Node *parent);
  void rotateRight(Node *child, Node *node, Node *parent);
  int height(Node *node);
  void rebalance(Node *node);
};
}
#endif // PAVT_BINARY_SEARCH_TREE_H_
