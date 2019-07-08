#ifndef TTREE_INCLUDE_PAVT_BASE_BINARY_TREE_H_
#define TTREE_INCLUDE_PAVT_BASE_BINARY_TREE_H_

#include <mutex>
#include <atomic>

#include <PaVT/Base/locknode.h>
#include <PaVT/lock_manager.h>

namespace pavt {
// Constants
const int MAXBF = 1; // Max Balance Factor
const int MINBF = -1; // Min Balance Factor
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
    int height;
    Node(const int& key) : _key(key) , height(0) {
      left = right = parent = nullptr;
    }
    virtual ~Node() {}

    int getKey() {
      return _key;
    }

    Node *get(int field) {
      if (field==0) return this->left;
      if (field==1) return this->right;
      if (field==2) return this;
    }
  };
 protected:
  Node* root;
};
} // namespace base
} //namespace pavt
#endif // TTREE_INCLUDE_PAVT_BASE_BINARY_TREE_H_
