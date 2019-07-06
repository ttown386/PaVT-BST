#ifndef PAVT_AVL_TREE_H_
#define PAVT_AVL_TREE_H_

#include <PaVT/bst.h>

namespace pavt {

class AVL : public BST {
 public:
  using BST::BST;
  void insert(const int& key);
  void remove(const int& key);
 protected:
  void rotateLeft(Node* child, Node* node, Node* parent);
  void rotateRight(Node* child, Node* node, Node* parent);
  int height(Node* node);
  void rebalance(Node* node);
};
}
#endif // PAVT_AVL_TREE_H_