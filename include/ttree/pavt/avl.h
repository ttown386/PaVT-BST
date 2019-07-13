#ifndef TTREE_INCLUDE_PAVT_PAVT_AVL_TREE_H_
#define TTREE_INCLUDE_PAVT_PAVT_AVL_TREE_H_ 

#include <ttree/pavt/bst.h>

namespace pavt {

const int MAXBF = 1; // Max Balance Factor
const int MINBF = -1; // Min Balance Factor

class AVL : public BST {
 public:
  class Node : public BST::Node {
   public:
    int height;
    Node(const int& key) : BST::Node(key) , height(0) {}
    ~Node() {}
  };
  using BST::BST;
  void Insert(const int& key);
  void Remove(const int& key);
  bool Contains(const int& key);

 protected:
  void RotateLeft(Node* child, Node* node, Node* parent);
  void RotateRight(Node* child, Node* node, Node* parent);
  int Height(Node* node);
  void Rebalance(Node* node);
};
}
#endif // TTREE_INCLUDE_PAVT_PAVT_AVL_TREE_H_