#include <iostream>

#include <ttree/sequential/bst.h>

namespace pavt {
namespace seq {

namespace {

BST::Node* find_min(BST::Node* node) {
  auto curr = node;
  auto next = curr->left;
  while(next != nullptr) { 
    curr = next; 
    next = next->left;
  }
  return curr;
}
} // namespace

BST::BST() {
  root = nullptr;
}

BST::~BST() {
  if (root != nullptr) delete root;
}

void  BST::Insert(const int& key) {
  BST::Node* new_node = new BST::Node(key);
  BST::Node* return_node = Insert(new_node);  
  if (return_node == nullptr) {
    delete new_node;
  }
}

void BST::Remove(const int& key) {
  auto balance_nodes = BST::Remove(root, key);
  delete balance_nodes;
}

bool BST::Contains(const int& key) {
  BST::Node* node = Traverse(this->root, key);
  if (node == nullptr) {
    return false;
  }
  return node->getKey() == key;
}

BST::Node* BST::Traverse(BST::Node* node, const int& key) {
  if (node == nullptr || node->getKey() == key) {
    return node;
  }
  BST::Node* curr = node;
  BST::Node* next = (curr->getKey() < key) ? curr->right : curr->left;
  while (next != nullptr) {
    curr = next;
    next = (curr->getKey() < key) ? curr->right : curr->left;
    if (curr->getKey() == key) return curr;
  }
  return curr;
}

BST::Node* BST::Insert(BST::Node* new_node) {
  BST::Node* curr = Traverse(root, new_node->getKey());
  if (curr == nullptr) {
    root = new_node;
    return new_node;
  }

  if (curr->getKey() == new_node->getKey()) {
    return nullptr; 
  }

  if (curr->getKey() < new_node->getKey()) {
    curr->right = new_node;
  } else {
    curr->left = new_node;
  }
  new_node->parent = curr;
  return curr;
}

std::pair<BST::Node*, BST::Node*>*
BST::Remove(Node* node, const int& key) {

  Node *toBalance1 = nullptr;
  Node *toBalance2 = nullptr;
  
  // Continually attempt removal until call is returned
  while (true) {

    // Grab node
    Node *curr = Traverse(node, key);

    // Already checked snapshots so return if current
    // node is not one to be deleted
    if (curr->getKey() != key) {
      return new std::pair<Node*, Node*>(nullptr, nullptr);
    }
    
    Node* parent = curr->parent;
    Node* leftChild = curr->left;
    Node* rightChild = curr->right;
    bool parentIsLarger = (parent->getKey() > key ? true : false);
    
    /*  A leaf node */
    if (leftChild== nullptr && rightChild == nullptr) {

      if (parentIsLarger && parent != nullptr) {
        parent->left = nullptr;
      } else {
        parent->right = nullptr;
      }
      toBalance1 = parent;

    } else if (leftChild==nullptr || rightChild==nullptr) {
    /* A node with at most 1 child */

      bool hasRightChild = leftChild == nullptr;
      Node* currChild = (hasRightChild) ? rightChild : leftChild;
      currChild = (hasRightChild) ? rightChild : leftChild;
      if (parent->left==curr) {
        parent->left = currChild;
      } else {
        parent->right = currChild;
      }
      currChild->parent = parent;
      toBalance1 = parent;
    } else {
     /* Node with where the right child's left node is null */
      if (rightChild->left == nullptr) {
        // Updated pointers
        rightChild->left = leftChild;
        leftChild->parent = rightChild;
        rightChild->parent = parent;

        if (parent->left==curr) {
          parent->left = rightChild;
        } else {
          parent->right =rightChild;
        }

        toBalance1 = rightChild; 

      } else {
        /* Hardest Case */

        Node *succ = find_min(rightChild->left);
        Node *succParent = succ->parent;
        // and rightSnap if it is not the right child
        Node *succRightChild = succ->right;

        succ->right = rightChild;
        rightChild->parent = succ;

        succ->left = leftChild;
        leftChild->parent = succ;
        succ->parent = parent;
        
        if (parentIsLarger) {
          parent->left = succ;
        } else {
          parent->right = succ;
        }
        succParent->left = succRightChild;

        if (succRightChild!=nullptr)
          succRightChild->parent = succParent;
        
        toBalance1 = succ;
        toBalance2 = succParent;
      }
    }
  }
  return new std::pair<Node*, Node*>(toBalance1, toBalance2);
}
} // namesapce seq
} // namespace pavt