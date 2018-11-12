//
// Created by ttown on 9/30/2018.
//

#include "BinarySearchTree.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stack>
#include <queue>
#include <stdio.h>
#include <thread>
#include <random>
#include <algorithm>
#include <sstream>

const int MAXBF = 1;
const int MINBF = -1;
const int iMin = std::numeric_limits<int>::min();
const int iMax = std::numeric_limits<int>::max();
const int LEFT = 0;
const int RIGHT = 1;
const int HERE = 2;

BinarySearchTree::BinarySearchTree(bool isAvl) {
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

BinarySearchTree::~BinarySearchTree() {
//  delete root;
}

Node *BinarySearchTree::getRoot() {
  return root;
}

Node *BinarySearchTree::traverse(Node *node, int const &data) {

  bool restart = false;

  while (true) {

    Node *curr = node;
    Node *parent = node->getParent();

    // traverse
    while (curr != nullptr) {

      if (curr->getData() == data) {
        curr->lock.lock();
        if (curr->mark) {
          restart = true;
          curr->lock.unlock();
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

    parent->lock.lock();
    // grab snapshot
    // check if restart is needed
    bool goLeft = (data < parent->getData() ? true : false);
    Node *snapShot = (goLeft? parent->leftSnap.load() : parent->rightSnap.load());
    if (parent->mark || (goLeft && (data <= snapShot->getData()))||
        (!goLeft &&(data >= snapShot->getData()))) {
      parent->lock.unlock();
      continue;
    }
    return parent;
  }
}

void BinarySearchTree::insert(int const &data) {

  // Otherwise we traverse
  while (true) {

    Node *curr = root;
    Node *child;
    int res = -1;
    while (true) {
      if (res==0) break;
      if (res < 0) {
        child = curr->getLeft();
      } else {
        child = curr->getRight();
      }
      if (child == nullptr) {
        break;
      }
      curr = child;
      int val = curr->getData();
      res = data - val;
    }

    if (res == 0) {
      return;
    }

    bool leftLast = res < 0;

    curr->lock.lock();
    if (curr->mark || (leftLast && curr->getLeft()!=nullptr) || (!leftLast && curr->getRight()!=nullptr)) {
      curr->lock.unlock();
      continue;
    }

    Node *snapshot = leftLast ? curr->leftSnap.load() : curr->rightSnap.load();
    if ((leftLast && (data <= snapshot->getData())) || 
        (!leftLast && (data >= snapshot->getData()))) {
      curr->lock.unlock();
      continue;
    }
    
    Node *newNode = new Node(data);
    newNode->setParent(curr);

    if (leftLast) {
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
    if (isAvl){
      // Update heights
      updateHeights(curr);
      rebalance(curr);
    }

    // Unlock
    curr->lock.unlock();
  }
}

void BinarySearchTree::remove(int const &data, int &thread_id) {

  int id = thread_id;
  while (true) {
    Node *node = root;
    Node *leftNode = maxSentinel;
    Node *rightNode = minSentinel;
    Node *child;
    int res = -1;
    while (true) {
      if (res==0) break;
      if (res < 0) {
        leftNode = node;
        child = node->getLeft();
      } else {
        rightNode = node;
        child = node->getRight();
      }
      if (child == nullptr) {
        break;
      }
      node = child;
      int val = node->getData();
      res = data - val;
    }
    if (res!=0) {
      bool leftLast = res < 0;
      Node *ref = leftLast ? node->leftSnap.load() : node->rightSnap.load();
      if (
        (leftLast && (data <= ref->getData())) ||
         (!leftLast && (data >= ref->getData()))) {
        continue;
      }
      return;
    }

    // Lock all nodes
    Node *parent = node->getParent();
    parent->lock.lock();
    if (node->getParent()!=parent) {
      parent->lock.unlock();
      if (node->mark) return;
      continue;
    }

    node->lock.lock();
    if (node->mark) {
      node->lock.unlock();
      parent->lock.unlock();
      return;
    }

    /*  A leaf node */
    Node *left = node->getLeft();
    Node *right = node->getRight();
    bool leftChild = parent->getLeft() == node;

    if (left == nullptr && right == nullptr) {
      rightNode = node->leftSnap.load();
      leftNode = node->rightSnap.load();
      node->mark = true;
      if (leftChild) {
        parent->setLeft(nullptr);
        parent->leftSnap = rightNode;
        rightNode->rightSnap = parent;
      } else {
        parent->setRight(nullptr);
        parent->rightSnap = leftNode;
        leftNode->leftSnap = parent;
      }
      node->lock.unlock();
      parent->lock.unlock();
      return;

      /* A node with at most 1 child */ 
    } else if (left==nullptr || right==nullptr) {
      Node *currChild = left==nullptr ? right : left;
      rightNode = node->leftSnap.load();
      leftNode = node->rightSnap.load();

      currChild->lock.lock();

      Node *snapshot =  left==nullptr ? leftNode : rightNode;
      // if the snapshot of curr is not its child then lock
      // that node as its path can be altered
      bool lockedSnap = false;
      if (snapshot!=currChild) {
        snapshot->lock.lock();
        lockedSnap = true;

      }
      // if the snapshot has changed restart
      if ((left==nullptr && snapshot->leftSnap.load()!=node) || 
          (left!=nullptr && snapshot->rightSnap.load()!=node) ||
          snapshot->mark) {

        if (lockedSnap) snapshot->lock.unlock(); // Released in reverse locking order
        currChild->lock.unlock();
        node->lock.unlock();
        parent->lock.unlock();
        continue;
      }

      node->mark = true;
      currChild = left==nullptr ? right : left;
      if (leftChild) {
        parent->setLeft(currChild);
      } else {
        parent->setRight(currChild);
      }

      currChild->setParent(parent);

      // TODO: Update Snaps
      rightNode->rightSnap = leftNode;
      leftNode->leftSnap = rightNode;

      // TODO: Unlock all
      if (lockedSnap) snapshot->lock.unlock(); // Released in reverse locking order
      currChild->lock.unlock();
      node->lock.unlock();
      parent->lock.unlock();

      // delete node;
      return;
    } else {

      left->lock.lock();
      right->lock.lock();

      rightNode = node->leftSnap.load();
      leftNode = node->rightSnap.load();

      bool lockedRightNode = false;
      if (rightNode != left) {
        rightNode->lock.lock();
        lockedRightNode = true;
      } 

      if (rightNode->rightSnap!=node || rightNode->mark) {
        if (lockedRightNode) rightNode->lock.unlock();
        right->lock.unlock();
        left->lock.unlock();
        node->lock.unlock();
        parent->lock.unlock();
        continue;
      }

      /* Node with where the right child's left node is null */
      if (right->getLeft() == nullptr) {
        node->mark = true;

        right->setLeft(left);
        left->setParent(right);
        right->setParent(parent);

        if (leftChild) {
          parent->setLeft(right);
        } else {
          parent->setRight(right);
        }

        // TODO: update snaps
        rightNode->rightSnap = leftNode;
        leftNode->leftSnap = rightNode;

        // TODO: Unlock all
        if (lockedRightNode) rightNode->lock.unlock();
        right->lock.unlock();
        left->lock.unlock();
        node->lock.unlock();
        parent->lock.unlock();
          
        return;
      } else {

        Node *succ = leftNode;
        Node *succParent = succ->getParent();

        // Try and lock succ parent
        bool lockedSuccessorParent = false;
        if (succParent!=right) {

          succParent->lock.lock();

          if (leftNode->getParent() != succParent || leftNode->mark) {
            succParent->lock.unlock();
            if (lockedRightNode) rightNode->lock.unlock();
            right->lock.unlock();
            left->lock.unlock();
            node->lock.unlock();
            parent->lock.unlock();
            continue;
          }

          lockedSuccessorParent = true;
        }
        // Try and lock succ
        succ->lock.lock();
        if (leftNode->leftSnap.load()!=node || leftNode->mark) {
          succ->lock.unlock();
          if (lockedSuccessorParent) succParent->lock.unlock();
          if (lockedRightNode) rightNode->lock.unlock();
          right->lock.unlock();
          left->lock.unlock();
          node->lock.unlock();
          parent->lock.unlock();
          continue;
        }

        Node *succRightChild = succ->getRight();
        if (succRightChild!=nullptr)  {

          succRightChild->lock.lock();

          Node *succRightSnapshot = succ->rightSnap.load();
          if (succRightSnapshot!=succRightChild) {
            succRightSnapshot->lock.lock();

            // Unlock all
            if (succRightSnapshot->leftSnap!=succ || succRightSnapshot->mark) {
              succRightSnapshot->lock.unlock();
              succRightChild->lock.unlock();
              succ->lock.unlock();
              if (lockedSuccessorParent) succParent->lock.unlock();
              if (lockedRightNode) rightNode->lock.unlock();
              right->lock.unlock();
              left->lock.unlock();
              node->lock.unlock();
              parent->lock.unlock();
              continue;
            }

            applyRemove(rightNode, node, parent, left, right, leftChild, succ, succParent, succRightChild, succRightSnapshot);
            succRightSnapshot->lock.unlock();
            succRightChild->lock.unlock();
            succ->lock.unlock();
            if (lockedSuccessorParent) succParent->lock.unlock();
            if (lockedRightNode) rightNode->lock.unlock();
            right->lock.unlock();
            left->lock.unlock();
            node->lock.unlock();
            parent->lock.unlock();
            return;

          } else {
            applyRemove(rightNode, node, parent, left, right, leftChild, succ, succParent, succRightChild, succRightSnapshot);
            succRightChild->lock.unlock();
            succ->lock.unlock();
            if (lockedSuccessorParent) succParent->lock.unlock();
            if (lockedRightNode) rightNode->lock.unlock();
            right->lock.unlock();
            left->lock.unlock();
            node->lock.unlock();
            parent->lock.unlock();
            return;
          }
        } else {
          Node *succRightSnapshot = succ->rightSnap.load();
          applyRemove(rightNode, node, parent, left, right, leftChild, succ, succParent, succRightChild, succRightSnapshot);
          succ->lock.unlock();
          if (lockedSuccessorParent) succParent->lock.unlock();
          if (lockedRightNode) rightNode->lock.unlock();
          right->lock.unlock();
          left->lock.unlock();
          node->lock.unlock();
          parent->lock.unlock();
          return;
        }
      }
    }
  }
}

void BinarySearchTree::applyRemove(Node *rightNode, 
                Node *node, 
                Node *parent, 
                Node *left, 
                Node *right, 
                bool leftChild, 
                Node *succ, 
                Node *succParent, 
                Node *succRight, 
                Node *succRightSnapshot) {

  node->mark = true;
  succ->setRight(right);
  right->setParent(succ);
  succ->setLeft(left);
  left->setParent(succ);
  succ->setParent(parent);

  if (leftChild) {
    parent->setLeft(succ);
  } else {
    parent->setRight(succ);
  }

  succParent->setLeft(succRight);

  succ->rightSnap = succRightSnapshot;
  succRightSnapshot->leftSnap = succ;

  if (succRight != nullptr) {
    succRight->setParent(succParent);
  }

  succ->leftSnap = rightNode;
  rightNode->rightSnap = succ;
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

void printSnaps(BinarySearchTree bst) {
  Node *start = bst.root;
  std::queue<Node *> q;
  q.push(start);

  while(!q.empty()) {

    Node *curr = q.front();
    q.pop();
    std::cout<<curr->getData()<<": ("<<curr->leftSnap.load()->getData()<<",";
    std::cout<<curr->rightSnap.load() ->getData()<<")\n";
    if (curr->getLeft()!=nullptr) q.push(curr->getLeft());
    if (curr->getRight()!=nullptr) q.push(curr->getRight());
  }
}

std::vector<int> init_list_ints(int num) {
  std::vector<int> vector;
  for (int i=0; i<num; i++) {
    vector.push_back(i);
  }
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(vector), std::end(vector), rng);
  return vector;
}

std::vector<Node *> init_list(int num) {
  std::vector<Node *> vector;
  for (int i=0; i<num; i++) {
    Node *n = new Node(i);
    vector.push_back(n);
  }
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(vector), std::end(vector), rng);
  return vector;
}

void routine_1(BinarySearchTree &bst, int id, int n_threads, 
              std::vector<int> keys) {

  int count = 0;
  for (int i=1*id; i<keys.size(); i+=n_threads) {
    // printf("Thread %d: insert #%d\n", id, keys.at(i));
    bst.insert(keys.at(i));
    // count++;
  }
  int increment = n_threads;
  for (int i=1*id; i<keys.size(); i+=increment) {
    // printf("Thread %d: remove #%d\n", id, keys.at(i));  
    bst.remove(keys.at(i), id);
    // count++;
  }
}

// // Routine for threads. Performs all ops
// void routine_4(BinarySearchTree &bst, std::vector<node *> list, 
//                int id, int n_threads, std::vector<int> ops) {

//   std::mt19937_64 eng{std::random_device{}()};
//   std::uniform_int_distribution<> dist{10, 100};

//   int count = 0;
//   int iter = id - 1;
//   int select = 0;
//   for (int i=0; i<ops.size(); i++) {
//     if (ops.at(i)==0) {
//       s.insert(list.at(iter));
//       iter+=n_threads;
//     } else if (ops.at(i) == 1) {
//       s.pop();
//     } else {
//       s.size();
//     }
//   }
// }

// // initialize order of ops
// std::vector<int> init_ops(int max, int insert, int remove, int contains) {
//   std::vector<int> ops;
//   for (int i=0; i<insert; i++) {
//     ops.push_back(0);
//   }
//   for (int i=0; i<remove; i++) {
//     ops.push_back(1);
//   }
//   for (int i=0; i<contains; i++) {
//     ops.push_back(2);
//   }
//   auto rng = std::default_random_engine {};
//   std::shuffle(std::begin(ops), std::end(ops), rng);
//   return ops;
// }

/**
 * Sequential Main
 */
// int main() {

//   BinarySearchTree bst;

//   bst.insert(7);
//   bst.insert(3);
//   bst.insert(12);
//   bst.insert(9);
//   bst.insert(1);
//   bst.insert(6);
//   bst.insert(14);
//   bst.insert(10);
//   bst.insert(11);

//   preOrderTraversal(bst);
//   inOrderTraversal(bst);
//   printSnaps(bst);
//   printf("\n");

//   printf("Remove 11\n");
//   bst.remove(11);
//   preOrderTraversal(bst);
//   inOrderTraversal(bst);
//   printSnaps(bst);
//   printf("\n");

//   printf("Remove 10\n");
//   bst.insert(11);
//   bst.remove(10);
//   preOrderTraversal(bst);
//   inOrderTraversal(bst);
//   printSnaps(bst);
//   printf("\n");

//   printf("Remove 12\n");
//   bst.remove(12);
//   preOrderTraversal(bst);
//   inOrderTraversal(bst);
//   printSnaps(bst);
//   printf("\n");

//   printf("Remove 7\n");
//   bst.remove(7);
//   preOrderTraversal(bst);
//   inOrderTraversal(bst);
//   printSnaps(bst);
//   printf("\n");
//   return 0;
// }

bool check (BinarySearchTree bst) {
  Node *curr = bst.minSentinel;
  int currVal = curr->getData();
  while (curr!=bst.maxSentinel) {
    curr = curr->rightSnap;
    int nextVal = curr->getData();
    if (nextVal <= currVal) return false;
    currVal = nextVal;
  }

  return true;
}


int main(int argc, char **argv) {
  
  if (argc!=2) {
    std::cout<<"Please enter correct number of arguments!"<<std::endl;
    return -1;
  }
  
  int n_threads = std::atoi(argv[1]);
  std::thread threads[n_threads];

  std::vector<int> keys = init_list_ints(200000);
  BinarySearchTree bst = BinarySearchTree();
  for (int i=0; i<n_threads; i++) {
    threads[i] = std::thread(routine_1, std::ref(bst), i, n_threads, std::ref(keys));  
  }
  for (int i=0; i<n_threads; i++) {
    threads[i].join();
  }

  std::cout<<"checking "<<check(bst)<<std::endl;

  // printf("Preorder :");
  // preOrderTraversal(bst);
  // printf("Inorder :");
  // inOrderTraversal(bst);
  // printf("Snaps: \n");
  // printSnaps(bst);
  // printf("\n");
}