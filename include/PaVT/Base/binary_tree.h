#ifndef TTREE_INCLUDE_PAVT_BASE_BINARY_TREE_H_
#define TTREE_INCLUDE_PAVT_BASE_BINARY_TREE_H_

#include <vector>

#include <PaVT/Base/locknode.h>
#include <PaVT/pavt/lock_manager.h>

namespace pavt {

const int iMin = std::numeric_limits<int>::min(); // Min Integer
const int iMax = std::numeric_limits<int>::max(); // Max Integer
const int LEFT = 0;
const int RIGHT = 1;
const int HERE = 2;

namespace base {

class BinaryTree {
 public:
  class Node {
   protected:
    int _key;
   public:
    Node* left, *right, *parent;
    Node(const int& key) : _key(key) { left = right = parent = nullptr; }
    virtual ~Node() {}
    inline int getKey() { return _key; }
    inline Node *get(int field) {
      if (field==0) return this->left;
      if (field==1) return this->right;
      if (field==2) return this;
    }
  };
  inline Node* GetRoot() { return root; }
  friend std::vector<int> InOrderTraversal(BinaryTree &bst);
  friend std::vector<int> PreOrderTraversal(BinaryTree &bst);

 protected:
  Node* root;
  void RotateLeft(Node* child, Node* node, Node* parent);
  void RotateRight(Node* child, Node* node, Node* parent);
};
} // namespace base
} //namespace pavt
#endif // TTREE_INCLUDE_PAVT_BASE_BINARY_TREE_H_
