//
// Created by ttown on 9/30/2018.
//

#ifndef TTREE_INCLUDE_PAVT_PAVT_BST_H_
#define TTREE_INCLUDE_PAVT_PAVT_BST_H_ 

#include <mutex>
#include <atomic>

#include <ttree/base/binary_tree.h>
#include <ttree/pavt/pavt_bst.h>
#include <ttree/pavt/lock_manager.h>

namespace pavt {

class BST: public PaVTBST {
 public:
  inline BST() {
    minSentinel = new PaVTBST::Node(iMin);
    maxSentinel = new PaVTBST::Node(iMax);

    maxSentinel->parent = (minSentinel);
    minSentinel->right = (maxSentinel);

    maxSentinel->leftSnap = minSentinel;
    minSentinel->rightSnap = maxSentinel;

    this->root = maxSentinel;
  }

  inline ~BST() { DeleteTree((Node*) root); }
  void Insert(const int& key);
  void Remove(const int& key);
  bool Contains(const int& key);
  
 private:
  void DeleteTree(Node* start);
};
} // namespace pavt
#endif // TTREE_INCLUDE_PAVT_PAVT_BST_H_
