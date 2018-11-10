//
// Created by ttown on 9/30/2018.
//

#include "BinarySearchTree.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stack>

const int MAXBF = 1;
const int MINBF = -1;
const int iMin = std::numeric_limits<int>::min();
const int iMax = std::numeric_limits<int>::max();
const int LEFT = 0;
const int RIGHT = 1;
const int HERE = 2;

BinarySearchTree::BinarySearchTree(bool isAvl) {
  this->isAvl = isAvl;
  this->root = new Node(iMax);
  Node *min = new Node(iMin);
  root->setLeft(min);
  min->setParent(root);
  root->setParent(nullptr);
  root->sentinel = true;
  min->sentinel = true;
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

int nextField(Node *node, int const &data) {

  // c1(node, data) = L
  if (data<node->getData()) return LEFT;

  // c2(node, data) = R
  if (data>node->getData()) return RIGHT;

  // c3(node, data) = null;
  return HERE;
}

Node *BinarySearchTree::traverse(Node *node, int const &data) {

  bool restart = false;
  while (true) {

    Node *curr = node;
    int nextField = nextField(curr, data);

    // traverse
    while (curr->get(nextField)!=nullptr) {

      curr = curr->get(nextField);

      nextField = nextField(curr, data);

      // We have found node
      if (nextField==HERE) {

        // If marked then break from first while loop and restart
        curr->lock.lock();
        if (curr->mark) {
          restart = true;
          curr->lock.unlock();
          break;
        }

        // Only executed if curr is not marked
        return curr;
      }
    }

    if (restart == true) {
      restart = false;
      continue;
    }

    curr->lock.lock();

    // grab snapshot
    // check if restart is needed

    return curr;
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

    /* A node with at most 1 child */
    if (leftChild==nullptr || rightChild==nullptr) {
      
      curr->mark = true;
      
      Node *currChild = (leftChild==nullptr) ? rightChild : leftChild;
      
      if (parent->getLeft()==curr) {
        parent->setLeft(currChild);
      } else {
        parent->setRight(currChild);
      }

      if (currChild!=nullptr)
        currChild->setParent(parent);

      // TODO: Update Snaps
      
      // TODO: Unlock all

      // delete curr;
      return;
    }

    /* Node with where the right child's left node is null */
    if (rightChild->getLeft() == nullptr) {
      
      curr->mark = true;

      rightChild->setLeft(leftChild);
      leftChild->setParent(rightChild);
      rightChild->setParent(parent);

      if (parent->getLeft()==curr) {
        parent->setLeft(rightChild);
      } else {
        parent->setRight(rightChild);
      }

      // TODO: update snaps
      
      // TODO: Unlock all
      
      return;
    }

    // TODO this will be changed with the concurrent version
    // Calls remove twice which will be redundant
    Node *successor = traverse(rightChild, data);

    /**
     * if failed locking successor, release locks and restart
     *
     * try lock on successor parent
     *
     * try lock on successor right child
     *
     * if either fail restart
     */

    Node *successorParent = successor->getParent();
    Node *successorRightChild = successor->getRight();

    successor->setRight(rightChild);
    rightChild->setParent(successor);

    successor->setLeft(leftChild);
    leftChild->setParent(successor);

    successor->setParent(parent);

    
    if (parent->getLeft()==curr) {
      parent->setLeft(successor);
    } else {
      parent->setRight(successor);
    }

    successorParent->setLeft(successorRightChild);

    if (successorRightChild!=nullptr)
      successorRightChild->setParent(successorParent);
    
    // Update Snaps

    // Unlock All

    // delete curr;
    return;
  }
}

bool BinarySearchTree::contains(int const &data) {
  bool restart = false;
  while (true) {

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
        return true;
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

    // get snapshot

    // check if key is valid for snapshot

    return false;
  }
}

void BinarySearchTree::updateHeights(Node *curr) {

  // Get a copy of curr to not lose its reference in
  // other calls
  Node *temp = curr;

  while (!temp->sentinel) {
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

/**
 *
 * UTILITY FUNCTIONS
 * 
 */

void inOrderTraversal(BinarySearchTree &bst) {

  std::stack<Node*> stack;

  // TODO: bst should be a constant reference so as to not alter input
  Node *curr = bst.getRoot();

  while (!stack.empty() || curr!=nullptr) {

    if (curr!=nullptr) {
      stack.push(curr);
      curr = curr->getLeft();
    } else {
      curr = stack.top();
      stack.pop();
      std::cout<<(curr->getData())<<" ";
      curr = curr->getRight();
    }
  }
  std::cout<<std::endl;
}
void preOrderTraversal(BinarySearchTree &bst) {

  std::stack<Node*> stack;

  Node *curr = bst.getRoot();
  stack.push(curr);

  while (!stack.empty()) {

    curr=stack.top();
    std::cout<<(curr->getData())<<" ";
    stack.pop();

    if (curr->getRight()) {
      stack.push(curr->getRight());
    }
    if (curr->getLeft()) {
      stack.push(curr->getLeft());
    }
  }
  std::cout<<std::endl;
}

int main() {

  BinarySearchTree bst;
  const int balanced[7] =  {4, 2, 6, 1, 3, 5, 7};

  for (int i=0; i<7; i++) {
    bst.insert(balanced[i]);
  }

  preOrderTraversal(bst);
  inOrderTraversal(bst);
  std::cout<<bst.contains(6)<<std::endl;
  std::cout<<bst.contains(24)<<std::endl;

  bst.remove(6);
  bst.remove(24);

  preOrderTraversal(bst);
  inOrderTraversal(bst);

  BinarySearchTree bst2(true);

  for (int i=0; i<10; i++) {
    bst.insert(i);
  }

  preOrderTraversal(bst);
  inOrderTraversal(bst);

  return 0;
}
