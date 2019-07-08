//
// Created by ttown on 9/30/2018.
//

#include <limits>
#include <iostream>
#include <algorithm>
#include <thread>

#include <PaVT/bst.h>

namespace pavt {
/**
 * PaVTBST::insert Insert new node into tree. If tree contains node
 * no node is inserted
 *
 * 
 * @param key key to be inserted into tree
 */
void BST::insert(const int& key) {
  Node* new_node = new Node(key);
  Node* return_node = BinarySearchTree::insert(new_node);
  if (return_node == nullptr) {
    delete new_node;
  } 
}


/**
 * PaVTBST::remove Removes node from tree. If node is not present then the
 * call returns. 
 * @param key The key to be removed from the tree
 */
void BST::remove(const int& key) {
  auto balance_nodes = BinarySearchTree::remove((Node*)root, key);
  delete balance_nodes;
}

void BST::DeleteTree(Node* start) {
  if (start->left != nullptr) DeleteTree((Node*) start->left);
  if (start->right != nullptr) DeleteTree((Node*) start->right);
  delete start;
}
} // namesapce pavt