//
// Created by ttown on 9/30/2018.
//

#ifndef CONCURRENTTREETRAVERALS_SERIALUTILS_H
#define CONCURRENTTREETRAVERALS_SERIALUTILS_H

#include "BinarySearchTree.h"
#include <list>

std::list<int> preOrderTraversal(BinarySearchTree &bst);
std::list<int> inOrderTraversal(BinarySearchTree &bst);
void printPostOrder(BinarySearchTree bst);


#endif //CONCURRENTTREETRAVERALS_SERIALUTILS_H
