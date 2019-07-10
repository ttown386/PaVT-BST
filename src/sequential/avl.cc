#include <iostream>
#include <stdlib.h>
#include <functional>

#include <PaVT/sequential/avl.h>

namespace pavt {
namespace seq {
namespace {

int height(AVL::Node* node) {
  return (node == nullptr) ? -1 : node->height;
}

void update_height(AVL::Node* node) {
  // Update the tree heights
  int leftHeight = height((AVL::Node*)node->left);
  int rightHeight = height((AVL::Node*)node->right);
  node->height = 1 + std::max(leftHeight, rightHeight);
}

void update_heights(AVL::Node* previous_root, AVL::Node* new_root) {
  update_height(previous_root);
  update_height(new_root);
}

int balance_factor(AVL::Node* left, AVL::Node* right) {
  int left_height = height(left);
  int right_height = height(right);
  return left_height - right_height;
}

int balance_factor(AVL::Node* node) {
  return balance_factor((AVL::Node*)node->left, (AVL::Node*)node->right);
}
} // namespace

void AVL::Insert(const int& key) {
  Node* new_node = new Node(key);
  BST::Node* return_node = BST::Insert(new_node);
  if (return_node == nullptr) {
    delete new_node;
  } else {
    Rebalance((AVL::Node*)return_node);
  }
}

void AVL::Remove(const int& key) {
  auto balance_nodes = BST::Remove(root, key);
  if (balance_nodes->first != nullptr) {
    Rebalance((AVL::Node*)balance_nodes->first);
    if (balance_nodes->second != nullptr) Rebalance((AVL::Node*)balance_nodes->second);
  } 
  delete balance_nodes;
}

// Rotates node to the left. Child becomes nodes parent.
void AVL::RotateLeft(Node *child, Node *node, Node *parent) {
  base::BinaryTree::RotateLeft(child, node, parent); 
  update_heights(child, node);
}

//Rotates node to the right. Child becomes nodes parent
void AVL::RotateRight(Node *child, Node *node, Node *parent) {
  base::BinaryTree::RotateRight(child, node, parent);
  update_heights(child, node);
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 */
void AVL::Rebalance(Node* node) {

  if (node==root) {
    return;
  }

  Node* parent = (Node*)node->parent;

  while(node!=root) {

    Node* left = (Node*)node->left;
    Node* right= (Node*)node->right;

    int leftHeight = height(left);
    int rightHeight = height(right);

    int currHeight = std::max(leftHeight, rightHeight) + 1;
    int prevHeight = node->height;

    int bf = leftHeight - rightHeight;
    if (currHeight != prevHeight) {
      node->height = currHeight;
    } else if (bf <= 1) {
      return;
    }

    Node* child;
    // The node's right subtree is too tall
    if (bf < MINBF) {
      child = right;

      int childBf = balance_factor(child);

      Node *grandChild = (Node*)child->left;
      // Need to do double rotation
      if (childBf > 0) {
        RotateRight(grandChild, child, node);
        RotateLeft(grandChild, node, parent);
        node = grandChild;

      } else {
        RotateLeft(child, node, parent);
        node = child;
      }

      // The node's left subtree is too tall
    } else if (bf > MAXBF) {
      child = left;

      int childBf = balance_factor(child);
      Node* grandChild = (Node*)child->right;

      if (childBf < 0) {
        RotateLeft(grandChild, child, node);
        RotateRight(grandChild, node, parent);
        node = grandChild;
      } else {
        RotateRight(child, node, parent);
        node = child;
      }
    } else {
      node = parent;
      parent = (Node*)node->parent;
    }
  }
} 
} // namespace seq
} // namespace pavt