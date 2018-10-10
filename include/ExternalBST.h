//
// Created by bbwor on 9/24/2018.
//

#ifndef CONCURRENTTREETRAVERALS_EXTERNALBST_H
#define CONCURRENTTREETRAVERALS_EXTERNALBST_H

#include "Node.h"

class ExternalBST {
 private:
  Node* root;
  Node* findMin(Node *root);

 public:
  ExternalBST();
  ~ExternalBST();
  bool insert(int const& key);
  bool remove(int const& key);
  bool contains(int const& key);
};

#endif //CONCURRENTTREETRAVERALS_EXTERNALBST_H
