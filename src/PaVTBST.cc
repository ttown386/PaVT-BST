//
// Created by ttown on 9/30/2018.
//

#include <limits>
#include <iostream>
#include <algorithm>
#include <thread>

#include <PaVT/Base/node.h>
#include <PaVT/PaVTBST.h>

namespace pavt {
/**
 * PaVTBST::insert Insert new node into tree. If tree contains node
 * no node is inserted
 *
 * 
 * @param key key to be inserted into tree
 */
void PaVTBST::insert(const int& key) {
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
void PaVTBST::remove(const int& key) {
  auto balance_nodes = BinarySearchTree::remove(root, key);
  delete balance_nodes;
}

// Rotates node to the left. Child becomes nodes parent.
void PaVTBST::rotateLeft(Node *child, Node *node, Node *parent) {

  // Grab the nodes right child
  Node *newRoot = child;

  // Give node the left child of the rotated node since the
  // key is greater than node
  Node *temp = newRoot->getLeft();
  node->setRight(temp);

  // The node's right child (temp) now moves up to take the place of
  // node
  newRoot->setLeft(node);

  // Update parents
  if(temp!=nullptr) temp->setParent(node);

  Node *rootParent = parent;

  if (rootParent->getRight() == node) {
    rootParent->setRight(newRoot);
  } else {
    rootParent->setLeft(newRoot);
  }

  newRoot->setParent(rootParent);
  node->setParent(newRoot);

  // Update the tree heights
  int leftHeight = height(node->getLeft());
  int rightHeight = height(node->getRight());
  node->setHeight(1 + std::max(leftHeight, rightHeight));

  int newRootLeftHeight = height(newRoot->getLeft());
  int newRootRightHeight = height(newRoot->getRight());
  newRoot->setHeight(1 + std::max(newRootLeftHeight, newRootRightHeight));
}

//Rotates node to the right. Child becomes nodes parent
void PaVTBST::rotateRight(Node *child, Node *node, Node *parent) {

  // Grab the nodes left child
  Node* newRoot = child;

  // Give node the left child of newRoot since the key
  // is less than node
  Node *temp = newRoot->getRight();
  node->setLeft(temp);

  // The new Root moves up to take the place of node
  // Now set newNodes right pointer to node
  newRoot->setRight(node);

  // Update parents
  if(temp!=nullptr) temp->setParent(node);

  Node *rootParent = parent;
  if (rootParent->getRight() == node) {
    rootParent->setRight(newRoot);
  } else {
    rootParent->setLeft(newRoot);
  }

  newRoot->setParent(rootParent);
  node->setParent(newRoot);

  // Update the tree heights
  int leftHeight = height(node->getLeft());
  int rightHeight = height(node->getRight());
  node->setHeight(1 + std::max(leftHeight, rightHeight));

  int newRootLeftHeight = height(newRoot->getLeft());
  int newRootRightHeight = height(newRoot->getRight());
  newRoot->setHeight(1 + std::max(newRootLeftHeight, newRootRightHeight));
}

/*
 * Returns the height of node
 */
int PaVTBST::height(Node *node) {
  return (node == nullptr) ? -1 : node->getHeight();
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 */
void PaVTBST::rebalance(Node *node) {

  if (node==root) {
    return;
  }

  Node *parent = node->getParent();

  while(node!=root) {

    // lock parent
    parent->lock.lock();
    if (node->getParent()!=parent) {
      parent->lock.unlock();
      if (node->mark) {
        return;
      }

      parent = node->getParent();
      continue;
    }

    // lock node
    node->lock.lock();
    if (node->mark) {
      node->lock.unlock();
      parent->lock.unlock();
      return;
    }

    Node *left = node->getLeft();
    Node *right= node->getRight();

    int leftHeight = height(left);
    int rightHeight = height(right);

    int currHeight = std::max(leftHeight, rightHeight) + 1;
    int prevHeight = node->getHeight();

    int bf = leftHeight - rightHeight;
    if (currHeight != prevHeight) {
      node->setHeight(currHeight);
    } else if (bf <= 1) {
      node->lock.unlock();
      parent->lock.unlock();
      return;
    }

    Node *child;
    // The node's right subtree is too tall
    if (bf < MINBF) {
      child = right;
      child->lock.lock();

      Node *childLeft = child->getLeft();
      Node *childRight = child->getRight();

      int childLeftHeight = height(childLeft);
      int childRightHeight = height(childRight);

      int childBf = childLeftHeight - childRightHeight;

      Node *grandChild = childLeft;

      // Need to do double rotation
      if (childBf > 0) {
        grandChild->lock.lock();
        rotateRight(grandChild, child, node);
        rotateLeft(grandChild, node, parent);
        child->lock.unlock();
        node->lock.unlock();
        grandChild->lock.unlock();
        parent->lock.unlock();

        node = grandChild;

      } else {
        rotateLeft(child, node, parent);
        node->lock.unlock();
        child->lock.unlock();
        parent->lock.unlock();

        node = child;
      }

      // The node's left subtree is too tall
    } else if (bf > MAXBF) {
      child = left;
      child->lock.lock();

      Node *childLeft = child->getLeft();
      Node *childRight = child->getRight();

      int childLeftHeight = height(childLeft);
      int childRightHeight = height(childRight);

      int childBf = childLeftHeight - childRightHeight;

      Node *grandChild = childRight;

      if (childBf < 0) {
        grandChild->lock.lock();

        rotateLeft(grandChild, child, node);
        rotateRight(grandChild, node, parent);
        node->lock.unlock();
        child->lock.unlock();
        grandChild->lock.unlock();
        parent->lock.unlock();

        node = grandChild;
      } else {

        rotateRight(child, node, parent);

        node->lock.unlock();
        child->lock.unlock();
        parent->lock.unlock();

        node = child;
      }

    } else {

      node->lock.unlock();
      parent->lock.unlock();

      // Traverse back up tree
      node = parent;
      parent = node->getParent();
    }
  }
}
} // namespace pavt