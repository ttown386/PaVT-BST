//
// Created by ttown on 9/30/2018.
//

#ifndef PAVT_BINARY_SEARCH_TREE_H_
#define PAVT_BINARY_SEARCH_TREE_H_ 

#include <mutex>
#include <atomic>

#include <PaVT/Base/binary_tree.h>
#include <PaVT/binary_search_tree.h>
#include <PaVT/lock_manager.h>

namespace pavt {

class BST: public BinarySearchTree {
 public:
  inline BST() {
    minSentinel = new BinarySearchTree::Node(iMin);
    maxSentinel = new BinarySearchTree::Node(iMax);

    maxSentinel->parent = (minSentinel);
    minSentinel->right = (maxSentinel);

    maxSentinel->leftSnap = minSentinel;
    minSentinel->rightSnap = maxSentinel;

    this->root = maxSentinel;
  }

  inline ~BST() { DeleteTree((Node*) root); }
  void insert(const int& key);
  void remove(const int& key);
 private:
  void DeleteTree(Node* start);
};
}
#endif // PAVT_BINARY_SEARCH_TREE_H_
