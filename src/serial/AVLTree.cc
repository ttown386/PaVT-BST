//
// Created by tylertownsend on 9/20/18.
//

#include <algorithm>

#include "../../include/AVLTree.h"

const int MAXBF = 1;
const int MINBF = -1;

AVLTree::AVLTree() {
  root = nullptr;
}

AVLTree::~AVLTree() {
  delete root;
}
/*!
 * Inserts data into this AVLTree. If the data is already contained in the
 * AVL tree then it will not be inserted. This insertion ensures that the
 * resulting tree will be balanced.
 *
 * @param data The key to be inserted into this Tree
 * @return A boolean indicating whether or not the node was inserted
 */
bool AVLTree::insert(int const &data) {

  root = insert(root, data);
  // TODO fix boolean return
  return true;
}

/*
 * Inserts the node and returns a reference to the node.
 * @param root
 * @return
 */
Node * AVLTree::insert(Node *node, int const &data) {

  // Place in appropriate spot
  if (node == nullptr) {
    return new Node(data);
  } else if (data > node->getData()) {
    node->setRight(insert(node->getRight(), data));
  } else if (data <  node->getData()) {
    node->setLeft(insert(node->getLeft(), data));
  } else {
    return node;
  }

  // update height of the subtree rooted here
  node->setHeight(1 + std::max(height(node->getLeft()), height(node->getRight())));

  // rebalance if necessary
  node = rebalance(node);
  return node;
}


void AVLTree::remove(int const &data) {
  root = remove(root, data);
}

Node* AVLTree::remove(Node *node, int const &data) {

  if (node == nullptr) {
    return nullptr;
  } else if (data < node->getData()) {
    node->setLeft(remove(node->getLeft(), data));
  } else if (data > node->getData()) {
    node->setRight(remove(node->getRight(), data));
  // if we've found the data
  } else {

    Node* curr = nullptr;

    // The node has no children
    if (node->getLeft() == nullptr && node->getRight()==nullptr) {
      delete node;
      return nullptr;

    // only has a right subtree
    } else if (node->getLeft() == nullptr) {

      curr = root->getRight();

      // change the nodes pointer to null
      node->setRight(nullptr);
      delete node;

      // return new subtree
      return curr;

    // Only has left subtree
    } else if (node->getRight() == nullptr) {

      curr = node->getLeft();

      // change nodes pointer to nulll
      node->setLeft(nullptr);
      delete node;

    // We get the data from the max node of the left subtree
    // Then we delete that subtree
    } else {

      // Get te data for the
      int copy = findMax(node->getLeft());
      node->setData(copy);

      // Remove the node
      remove(node->getLeft(), copy);
    }
  }

  // update the height of the subtree rooted here
  node->setHeight(1 + std::max(height(node->getRight()), height(node->getLeft())));

  // rebalance the tree if necessary
  node = rebalance(node);
}


bool AVLTree::contains(int const &data) {
  Node* curr = root;
  return contains(curr, data);
}

bool AVLTree::contains(Node *node, int const &data) {

  if (root == nullptr) {
    return false;
  } else if (data < node->getData()){
    return contains(node->getLeft(), data);
  } else if (data > node->getData()) {
    return contains(node->getRight(), data);
  } else {
    return true;
  }
}


/*
 * Returns the maximum value in the subtree rooted at node
 * @param node The root of the subtree
 * @return
 */
int AVLTree::findMax(Node* node) {

  Node* curr = node;

  while (curr!=nullptr) {
    curr->setRight(curr->getRight());
  }

  return curr->getData();
}

/*
 * Returns the height of node
 */
int AVLTree::height(Node *node) {
  return (node == nullptr) ? -1 : node->getHeight();
}

/*
 * Returns the balance factor of node
 * @param node
 * @return
 */
int AVLTree::balanceFactor(Node *node) {
  return height(node->getLeft()) - height(node->getRight());
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 * @return
 */
Node *AVLTree::rebalance(Node *node) {

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
}

/*
 * Performs a single left rotation specified at node
 * @param node The node at which the rotation is performed
 * @return     The node that will replace node
 */
Node* AVLTree::rotateLeft(Node *node) {

  // Grab the nodes right child
  Node* newRoot = node->getRight();

  // Give node the left child of the rotated node since the
  // key is greater than node
  node->setRight(newRoot->getLeft());

  // The node's right child (temp) now moves up to take the place of
  // node
  newRoot->setLeft(node);

  // Update the tree heights
  node->setHeight(1 + std::max(height(node->getLeft()), height(node->getRight())));
  newRoot->setHeight(1 + std::max(height(newRoot->getLeft()), height(newRoot->getRight())));

  return newRoot;
}


/*
 * Performs a single right rotation specified at node
 * @param node The node at which the rotation is performed
 * @return     The node that will replace node
 */
Node* AVLTree::rotateRight(Node *node) {

  // Grab the nodes left child
  Node* newRoot = node->getLeft();

  // Give node the left child of newRoot since the key
  // is less than node
  node->setLeft(newRoot->getRight());

  // The new Root moves up to take the place of node
  // Now set newNodes right pointer to node
  newRoot->setRight(node);

  // Update the tree heights
  node->setHeight(1 + std::max(height(node->getLeft()), height(node->getRight())));
  newRoot->setHeight(1 + std::max(height(newRoot->getLeft()), height(newRoot->getRight())));

  return newRoot;
}




