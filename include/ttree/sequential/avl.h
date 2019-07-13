#ifndef TTREE_INCLUDE_PAVT_SEQUENTIAL_AVL_H_
#define TTREE_INCLUDE_PAVT_SEQUENTIAL_AVL_H_

#include <ttree/sequential/bst.h>

namespace pavt {
namespace seq {

const int MAXBF = 1; // Max Balance Factor
const int MINBF = -1; // Min Balance Factor

class AVL : public BST {
 public:
  class Node : public BST::Node {
   public:
    int height;
    inline Node(const int key) : BST::Node(key) {
      height = 0;
    }
  };

 protected:
  void Rebalance(Node* node);
  void RotateLeft(Node* child, Node* node, Node* parent);
  void RotateRight(Node* child, Node* node, Node* parent);

 public:
  using BST::BST;
  void Insert(const int& key);
  void Remove(const int& key);
};
} // namespace seq
} // namespace pavt
#endif // TTREE_INCLUDE_PAVT_SEQUENTIAL_AVL_H_