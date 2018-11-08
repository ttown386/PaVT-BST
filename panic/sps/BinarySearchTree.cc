//
// Created by ttown on 9/30/2018.
//

#include "../../include/BinarySearchTree.h"
#include <algorithm>
#include <iostream>
#include <limits>

const int MAXBF = 1;
const int MINBF = -1;
const int iMin = std::numeric_limits<int>::min();
const int iMax = std::numeric_limits<int>::max();

namespace utils {
Node *findMin(Node *node);
}

Node* utils::findMin(Node *node) {
  if(node->getLeft() == nullptr) {
    node = node->getLeft();
  }
  while(node->getLeft()->getLeft() != nullptr) {
    node = node->getLeft();
  }
  //If the node we are returning has items attached to it make sure to not trim those off
  //We do so by setting our left node to their right node
  if(node->getLeft()->getRight() != nullptr) {
    node->setLeft(node->getLeft()->getRight());
  }
  return node->getLeft();
}

BinarySearchTree::BinarySearchTree(bool isAvl) {
  this->isAvl = isAvl;
  this->root = new Node(iMax);
  Node *min = new Node(iMin);
  root->setLeft(min);
  min->setParent(root);
}

BinarySearchTree::~BinarySearchTree() {
//  delete root;
}

Node *BinarySearchTree::getRoot() {
  return root;
}

void updateSnaps(Node *start, Node *toUpdate, Node *parent) {
  if (toUpdate->getData()==iMin || toUpdate->getData()==iMax)
    return;
  
}

void updateSnaps(Node *node) {
  updateSnaps(node, node, node->getParent());
}

Node *BinarySearchTree::traverse(Node *node, int const &data) {
  bool restart = false;
  while (true) {
    // Otherwise we traverse

    Node *curr = root;
    Node *parent = nullptr;

    // traverse
    while (curr != nullptr) {

      // We have a duplicate
      if (curr->getData() == data) {
        if (curr->mark) {
          restart = true;
          break;
        }
        return curr;
      }

      // update parent;
      parent = curr;

      // traverse to next child
      bool parentIsLarger = data < parent->getData();
      curr = (parentIsLarger ? curr->getLeft() : curr->getRight());
    }

    if (restart == true) {
      restart = false;
      continue;
    }

    return (parent == nullptr ? root : parent);
  }
}

void BinarySearchTree::insert(int const &data) {

  // Otherwise we traverse
  Node *curr = traverse(root, data);

  // We have a duplicate
  // TODO incorperate for External BST
  if (curr->getData()==data ||
    (data > curr->getData() && curr->getRight()!=nullptr) ||
    (data < curr->getData() && curr->getLeft()!=nullptr)) {
    // TODO: unlock
    return;
  }

  // update parent;
  Node *newNode = new Node(data);
  newNode->setParent(curr);
  newNode->setHeight(0);

  // If the parent is larger than the child, set the left pointer
  // of parent to be the new node
  // traverse to next child
  bool parentIsLarger = data < curr->getData();
  if (parentIsLarger) {
    curr->setLeft(newNode);
  } else {
    curr->setRight(newNode);
  }

  //TODO: Update Snaps

  // Perform AVL rotations if applicable
  if (isAvl){
    // Update heights
    updateHeights(curr);
    rebalance(curr);
  }

  //TODO: Unlock
}

void BinarySearchTree::remove(int const &data) {

  while (true) {
    Node *curr = traverse(root, data);

    // TODO return false here
    if (curr==nullptr || curr->getData()!= data) {
      return;
    }

    // Todo: Lock all nodes

    Node *parent = curr->getParent();
    Node *leftChild = curr->getLeft();
    Node *rightChild = curr->getRight();

    // Easy removal
    if (leftChild==nullptr || rightChild==nullptr) {
      Node *currChild = (leftChild==nullptr) ? rightChild : leftChild;
      // TODO remove nullptr and incorperate sentinel node
      if (parent!=nullptr && parent->getLeft()==curr) {
        parent->setLeft(currChild);
      } else if (parent!=nullptr ){
        parent->setRight(currChild);
      }
      // Call update Snaps here instead of updating child's parent pointer
      currChild->setParent(parent);
      delete curr;
      return;
    } else {
      // TODO this will be changed with the concurrent version
      // Calls remove twice which will be redundant
      Node *min = utils::findMin(rightChild);
      // TODO remove nullptr and incorperate sentinel node
      if (parent!=nullptr && parent->getLeft()==curr) {
        parent->setLeft(min);
      } else if (parent!=nullptr){
        parent->setRight(min);
      }
      // Finish updating all pointers at old location
      Node *minOldParent = min->getParent();

      if (min!=rightChild) {
        // we know minsOldParent is greater than it
        minOldParent->setLeft(nullptr);

        rightChild->setParent(min);
        leftChild->setParent(min);

        min->setRight(rightChild);
      }
      // do this no matter what
      min->setParent(parent);
      min->setLeft(leftChild);
      delete curr;
      return;
    }
  }
}

bool BinarySearchTree::contains(int const &data) {
  return false;
}

void BinarySearchTree::updateHeights(Node *curr) {

  // Get a copy of curr to not lose its reference in
  // other calls
  Node *temp = curr;

  while (temp!=nullptr) {
    // update height of the subtree rooted here
    Node *left = temp->getLeft();
    Node *right = temp->getRight();
    int maxSubHeight = std::max(height(left), height(right));
    temp->setHeight(1 + maxSubHeight);

    // go to the next parent
    temp = temp->getParent();
  }
}

Node *BinarySearchTree::rotateLeft(Node *node) {

  // Grab the nodes right child
  Node *newRoot = node->getRight();

  // Give node the left child of the rotated node since the
  // key is greater than node
  Node *temp = newRoot->getLeft();
  node->setRight(temp);

  // The node's right child (temp) now moves up to take the place of
  // node
  newRoot->setLeft(node);

  // Update parents
  if(temp!=nullptr) temp->setParent(node);

  Node *rootParent = node->getParent();
  if (rootParent!=nullptr) {
    if (rootParent->getRight() == node) {
      rootParent->setRight(newRoot);
    } else {
      rootParent->setLeft(newRoot);
    }
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

  return newRoot;
}

Node *BinarySearchTree::rotateRight(Node *node) {

  // Grab the nodes left child
  Node* newRoot = node->getLeft();

  // Give node the left child of newRoot since the key
  // is less than node
  Node *temp = newRoot->getRight();
  node->setLeft(temp);

  // The new Root moves up to take the place of node
  // Now set newNodes right pointer to node
  newRoot->setRight(node);

  // Update parents
  if(temp!=nullptr) temp->setParent(node);

  Node *rootParent = node->getParent();
  if (rootParent!=nullptr) {
    if (rootParent->getRight() == node) {
      rootParent->setRight(newRoot);
    } else {
      rootParent->setLeft(newRoot);
    }
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

  return newRoot;
}

/*
 * Returns the height of node
 */
int BinarySearchTree::height(Node *node) {
  return (node == nullptr) ? -1 : node->getHeight();
}

/*
 * Returns the balance factor of node
 * @param node
 * @return
 */
int BinarySearchTree::balanceFactor(Node *node) {
  int hLeft = height(node->getLeft());
  int hRight = height(node->getRight());
  return hLeft - hRight;
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 * @return
 */
void BinarySearchTree::rebalance(Node *node) {

  // get balance factor
  int bf = balanceFactor(node);

  // The node's right subtree is too tall
  if (bf < MINBF) {

    // If the node's right subtree is left heavy, then
    // the subtree must be rotated to the right
    if (balanceFactor(node->getRight()) > 0)
      node->setRight(rotateRight(node->getRight()));

    // We rotate left when the node's right subtree is right- heavy
    // This will also account if the node's right node is also has
    // and right-heavy subtree.
    node = rotateLeft(node);

    // The node's left subtree is too tall
  } else if (bf > MAXBF) {

    // If the node's left subtree is right heavy, then
    // the subtree must be rotated to the left
    if (balanceFactor(node->getLeft()) < 0)
      node->setLeft(rotateLeft(node->getLeft()));

    // We rotate right when the node's left subtree is left-heavy
    // This will also account if the node's left node is also has
    // and left-heavy subtree.
    node = rotateRight(node);
  }
  // TODO : make this iterative
  if (node->getParent()!=nullptr) {
    rebalance(node->getParent());
  } else {
    root = node;
  }
}
