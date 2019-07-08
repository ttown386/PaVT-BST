#ifndef PAVT_AVL_TREE_H_
#define PAVT_AVL_TREE_H_

#include <PaVT/bst.h>

namespace pavt {

class AVL : public BST {
 public:
  class Node : public BST::Node {
   public:
    int height;
    Node(const int& key) : BST::Node(key) , height(0) {}
    ~Node() {}
  };
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