//
// Created by Robert on 9/23/2018.
//

#ifndef CONCURRENTTREETRAVERALS_INTERNALBST_H
#define CONCURRENTTREETRAVERALS_INTERNALBST_H

#include "Node.h"

class InternalBST {
 private:
  Node* root;
  Node* findMin(Node *root);

 public:
  InternalBST();
  ~InternalBST();
  bool insert(int const& key);
  bool remove(int const& key);
  bool contains(int const& key);
};


#endif //CONCURRENTTREETRAVERALS_INTERNALBST_H
