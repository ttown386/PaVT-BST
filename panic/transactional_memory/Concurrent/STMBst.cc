//
// Created by ttown on 9/30/2018.
//

#include "STMBst.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stack>
#include <queue>
#include <stdio.h>
#include <thread>
#include <random>
#include <cstdlib>
#include <sstream>

const int MAXBF = 1;
const int MINBF = -1;
const int iMin = std::numeric_limits<int>::min();
const int iMax = std::numeric_limits<int>::max();
const int LEFT = 0;
const int RIGHT = 1;
const int HERE = 2;

STMBst::STMBst(bool isAvl) {
  // AVL condition
  this->isAvl = isAvl;

  // Bounds
  minSentinel = new Node(iMin);
  maxSentinel = new Node(iMax);

  // tree structure
  maxSentinel->setParent(minSentinel);
  minSentinel->setRight(maxSentinel);

  // Snapshots
  maxSentinel->leftSnap = minSentinel;
  minSentinel->rightSnap = maxSentinel;

  // Sentinel Conditions
  maxSentinel->sentinel = true;
  minSentinel->sentinel = true;

  this->root = maxSentinel;
}

STMBst::~STMBst() {
//  delete root;
}

Node *STMBst::getRoot() {
  return root;
}

int STMBst::nextField(Node *node, int const &data) {

  // c1(node, data) = L
  if (data<node->getData()) return LEFT;

  // c2(node, data) = R
  if (data>node->getData()) return RIGHT;

  // c3(node, data) = null;
  return HERE;
}

Node *STMBst::traverse(Node *node, int const &data) {
  bool restart = false;
  while (true) {

    Node *curr = root;
    int field = nextField(curr, data);

    // traverse
    Node *next = curr->get(field);
    while (next != nullptr) {

      curr = next;

      field = nextField(curr, data);

      // We have found node
      if (field == HERE) {
        // std::cout<<"found node"<<std::endl;
        // std::cout<<"locked node"<<std::endl;
        // If marked then break from first while loop and restart
        if (curr->mark) {
          restart = true;
          break;
        }

        // Only executed if curr is not marked
        return curr;
      }
      next = curr->get(field); 
    }

    if (restart == true) {
      restart = false;
      continue;
    }
    return curr;
  }
}

void STMBst::insert(int const &data) {

  // Otherwise we traverse
  while (true) {
    Node *curr = traverse(root, data);
    __transaction_atomic{

      // check if the node
      if (curr->getData() == data && !curr->mark) {
        return;
      }

      // grab snapshot
      // check if restart is needed
      bool goLeft = (data < curr->getData() ? true : false);
      Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
      if (curr->mark ||
        (goLeft && (data <= snapShot->getData())) ||
        (!goLeft && (data >= snapShot->getData()))) {
        continue;
      }

      // No longer a leaf node
      if (
        (data > curr->getData() && curr->getRight() != nullptr) ||
        (data < curr->getData() && curr->getLeft() != nullptr)) {
        continue;
      }

      Node *newNode = new Node(data);
      newNode->setParent(curr);

      // Copy snaps from parent
      bool parentIsLarger = data < curr->getData();
      Node *snapshot = (parentIsLarger ? curr->leftSnap : curr->rightSnap);

      if (parentIsLarger) {
        newNode->leftSnap = snapshot;
        newNode->rightSnap = curr;

        snapshot->rightSnap = newNode;
        curr->leftSnap = newNode;

        curr->setLeft(newNode);
      } else {
        newNode->leftSnap = curr;
        newNode->rightSnap = snapshot;

        snapshot->leftSnap = newNode;
        curr->rightSnap = newNode;

        curr->setRight(newNode);
      }
      // Perform AVL rotations if applicable
    }

    // std::cout<<"rebalance"<<std::endl;
    __transaction_atomic {
      if (isAvl) rebalance(curr);
    }
  }
}

void STMBst::remove(int const &data) {

  Node *maxSnapNode;
  Node *minSnapNode;

  while (true) {
    
    Node *curr = traverse(root, data);

    // Apply atomic transaction
    __transaction_atomic{

      bool goLeft = (data < curr->getData() ? true : false);
      Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
      if (curr->mark ||
        (goLeft && (data <= snapShot->getData())) ||
        (!goLeft && (data >= snapShot->getData()))) {
        continue;
      }

      // Lock all nodes
      Node *parent = curr->getParent();

      //Some other thread has gone and changed things around
      //Got to check if we already got removed otherwise unlock restart
      if (parent != curr->getParent()) {
        if (curr->mark) {
          return;
        }
        continue;
      }

      // Already checked snapshots so return if current
      // node is not one to be deleted
      if (curr->getData() != data) {
        return;
      }

      // printf("%d: 1\n", id);
      /*  A leaf node */
      Node *leftChild = curr->getLeft();
      Node *rightChild = curr->getRight();
      bool parentIsLarger = (parent->getData() > data ? true : false);

      if (leftChild == nullptr && rightChild == nullptr) {

        //One possible atmoic segment
        maxSnapNode = curr->rightSnap;
        minSnapNode = curr->leftSnap;
        curr->mark = true;

        if (parentIsLarger) {
          parent->setLeft(nullptr);
          parent->leftSnap = minSnapNode;
          minSnapNode->rightSnap = parent;
        } else {
          parent->setRight(nullptr);
          parent->rightSnap = maxSnapNode;
          maxSnapNode->leftSnap = parent;
        }

        if (isAvl) rebalance(parent);
        return;
      }

      /* A node with at most 1 child */
      if (leftChild == nullptr || rightChild == nullptr) {

        bool hasRightChild = leftChild == nullptr;
        Node *currChild = (hasRightChild) ? rightChild : leftChild;

        //Another possible section
        minSnapNode = curr->leftSnap;
        maxSnapNode = curr->rightSnap;

        Node *snapshot = (hasRightChild) ? maxSnapNode : minSnapNode;

        // if the snapshot has changed restart
        // Atomic PlaceHolder
        if ((hasRightChild && snapshot->leftSnap != curr) ||
          (!hasRightChild && snapshot->rightSnap != curr) ||
          snapshot->mark) {
          continue;
        }

        curr->mark = true;
        currChild = (hasRightChild) ? rightChild : leftChild;
        if (parent->getLeft()==curr) {
          parent->setLeft(currChild);
        } else {
          parent->setRight(currChild);
        }

        currChild->setParent(parent);

        // TODO: Update Snaps
        minSnapNode->rightSnap = maxSnapNode;
        maxSnapNode->leftSnap = minSnapNode;

        if (isAvl) rebalance(parent);
        return;
      }

      /* Hard Cases */
      minSnapNode = curr->leftSnap;
      maxSnapNode = curr->rightSnap;

      if (minSnapNode->rightSnap != curr || minSnapNode->mark) {
        continue;
      }

      /* Node with where the right child's left node is null */
      // Atomic PlaceHolder
      if (rightChild->getLeft() == nullptr) {
        curr->mark = true;

        rightChild->setLeft(leftChild);
        leftChild->setParent(rightChild);
        rightChild->setParent(parent);

        if (parent->getLeft() == curr) {
          parent->setLeft(rightChild);
        } else {
          parent->setRight(rightChild);
        }

        minSnapNode->rightSnap = maxSnapNode;
        maxSnapNode->leftSnap = minSnapNode;

        if (isAvl) rebalance(rightChild);

        return;
      }

      Node *succ = maxSnapNode;
      Node *succParent = succ->getParent();

      if (succParent != rightChild) {
        if (maxSnapNode->getParent() != succParent || maxSnapNode->mark) {
          continue;
        }
      }

      if (maxSnapNode->leftSnap != curr || maxSnapNode->mark) {
        continue;
      }

      Node *succRightChild = succ->getRight();
      Node *succRightSnapshot = succ->rightSnap;

      if (succRightChild != nullptr) {
        succRightSnapshot = succ->rightSnap;
        if (succRightSnapshot->leftSnap != succ || succRightSnapshot->mark) {
          continue;
        }
      }
      // Mark the node
      curr->mark = true;

      succ->setRight(rightChild);
      rightChild->setParent(succ);

      succ->setLeft(leftChild);
      leftChild->setParent(succ);

      succ->setParent(parent);

      if (parentIsLarger) {
        parent->setLeft(succ);
      } else {
        parent->setRight(succ);
      }

      succParent->setLeft(succRightChild);
      if (succRightChild != nullptr)
        succRightChild->setParent(succParent);

      // Update Snaps
      succ->rightSnap = succRightSnapshot;
      succRightSnapshot->leftSnap = succ;
      // Atomic PlaceHolder
      succ->leftSnap = minSnapNode;
      minSnapNode->rightSnap = succ;

      if (isAvl) {
        rebalance(succ);
        rebalance(succParent);
      } 
      return;
    }
  }
}

bool STMBst::contains(int const &data) {
  bool restart = false;
  while (true) {

    Node *curr = root;
    int field = nextField(curr, data);

    // traverse
    Node *next = curr->get(field);
    while (next != nullptr) {

      curr = next;

      field = nextField(curr, data);

      // We have found node
      if (field == HERE) {
        // std::cout<<"found node"<<std::endl;
        // std::cout<<"locked node"<<std::endl;
        // If marked then break from first while loop and restart
        if (curr->mark) {
          restart = true;
          break;
        }

        // Only executed if curr is not marked
        return true;
      }
      next = curr->get(field);
    }

    if (restart == true) {
      restart = false;
      continue;
    }

    // grab snapshot
    // check if restart is needed
    bool goLeft = (data < curr->getData() ? true : false);
    Node* snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
    if (curr->mark || (goLeft && (data < snapShot->getData())) ||
      (!goLeft && (data > snapShot->getData()))) {
      continue;
    }
    return false;
  }
}


void STMBst::rotateLeft(Node *child, Node *node, Node *parent) {

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

void STMBst::rotateRight(Node *child, Node *node, Node *parent) {

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
int STMBst::height(Node *node) {
  return (node == nullptr) ? -1 : node->getHeight();
}


void STMBst::rebalance(Node *node) {

  if (node==root) {
    return;
  }

  Node *parent = node->getParent();

  while(node!=root) {

    // lock parent
    if (node->getParent()!=parent) {
      if (node->mark) {
        return;
      }
      parent = node->getParent();
      continue;
    }
    // std::cout<<"b\n";
    if (node->mark) {
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
      return;
    }

    Node *child;
    // The node's right subtree is too tall
    if (bf < MINBF) {
      child = right;

      Node *childLeft = child->getLeft();
      Node *childRight = child->getRight();

      int childLeftHeight = height(childLeft);
      int childRightHeight = height(childRight);

      int childBf = childLeftHeight - childRightHeight;

      Node *grandChild = childLeft;

      if (childBf > 0) {
        rotateRight(grandChild, child, node);
        rotateLeft(grandChild, node, parent);
        node = grandChild;

      } else {
        rotateLeft(child, node, parent);
        node = child;
      }

      // The node's left subtree is too tall
    } else if (bf > MAXBF) {
      child = left;

      Node *childLeft = child->getLeft();
      Node *childRight = child->getRight();

      int childLeftHeight = height(childLeft);
      int childRightHeight = height(childRight);

      int childBf = childLeftHeight - childRightHeight;

      Node *grandChild = childRight;

      if (childBf < 0) {
        rotateLeft(grandChild, child, node);
        rotateRight(grandChild, node, parent);
        node = grandChild;
      } else {
        rotateRight(child, node, parent);
        node = child;
      }

    } else {
      // Traverse back up tree
      node = parent;
      parent = node->getParent();
    }
  }
}


/**
 *
 * UTILITY FUNCTIONS
 * 
 */

void inOrderTraversal(STMBst &bst) {

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

void preOrderTraversal(STMBst &bst) {

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

void printSnaps(STMBst bst) {
  Node *start = bst.root;
  std::queue<Node *> q;
  q.push(start);

  while(!q.empty()) {

    Node *curr = q.front();
    q.pop();
    std::cout<<curr->getData()<<": ("<<curr->leftSnap->getData()<<",";
    std::cout<<curr->rightSnap->getData()<<")\n";
    if (curr->getLeft()!=nullptr) q.push(curr->getLeft());
    if (curr->getRight()!=nullptr) q.push(curr->getRight());
  }
}

std::vector<int> init_list_ints(int num) {
  std::vector<int> vector;
  for (int i = 0; i<num; i++) {
    vector.push_back(i);
  }
  auto rng = std::default_random_engine{};
  std::shuffle(std::begin(vector), std::end(vector), rng);
  return vector;
}

std::vector<Node *> init_list(int num) {
  std::vector<Node *> vector;
  for (int i = 0; i<num; i++) {
    Node *n = new Node(i);
    vector.push_back(n);
  }
  auto rng = std::default_random_engine{};
  std::shuffle(std::begin(vector), std::end(vector), rng);
  return vector;
}

bool check(STMBst bst) {
  Node *curr = bst.minSentinel;
  int currVal = curr->getData();
  while (curr != bst.maxSentinel) {
    curr = curr->rightSnap;
    int nextVal = curr->getData();
    if (nextVal <= currVal) return false;
    currVal = nextVal;
  }

  return true;
}

void routine_1(STMBst &bst, int id, int n_threads,
  std::vector<int> keys) {

  int count = 0;
  // std::cout<<"before"<<std::endl;
  for (int i = 1 * id; i<keys.size(); i += n_threads) {
    // printf("Thread %d: insert #%d\n", id, count);
    bst.insert(keys.at(i));
    // std::cout<<"insert"<<std::endl;
    // count++;
  }
  // std::cout<<"after"<<std::endl;
  int increment = n_threads;
  // std::cout<<"here"<<std::endl;
  for (int i = 1 * id; i<keys.size() / 2; i++) {
    // std::cout<<i<<std::endl;
    bst.remove(keys.at(i));
    // printf("Thread %d: remove #%d\n", id, count);
    // count++;
  }
}

int main(int argc, char **argv) {

  if (argc != 2) {
    std::cout << "Please enter correct number of arguments!" << std::endl;
    return -1;
  }

  int n_threads = std::atoi(argv[1]);
  std::thread threads[n_threads];

  std::vector<int> keys = init_list_ints(200000);
  
  STMBst bst = STMBst(true);
  for (int j=0; j<1; j++) {
    for (int i = 0; i<n_threads; i++) {
      threads[i] = std::thread(routine_1, std::ref(bst), i, n_threads, std::ref(keys));
    }
    for (int i = 0; i<n_threads; i++) {
      threads[i].join();
    }
  }

  // std::cout<<"check "<<check(bst)<<std::endl;
  // printf("Preorder :");
  // preOrderTraversal(bst);
  // printf("Inorder :");
  // inOrderTraversal(bst);
  // printf("Snaps: \n");
  // printSnaps(bst);
  // printf("\n");
}