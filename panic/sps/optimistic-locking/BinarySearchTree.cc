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

int BinarySearchTree::nextField(Node *node, int const &data) {

  // c1(node, data) = L
  if (data<node->getData()) return LEFT;

  // c2(node, data) = R
  if (data>node->getData()) return RIGHT;

  // c3(node, data) = null;
  return HERE;
}

void BinarySearchTree::updateSnaps(Node *node) {

  Node *parent = node->getParent();
  int field = nextField(parent, node->getData());

  if (field == LEFT){
    (parent->leftSnap).load()->rightSnap = node;
    parent->leftSnap = node;
  } else {
    (parent->rightSnap).load()->leftSnap = node;
    parent->rightSnap = node;
  }
  
}

Node *BinarySearchTree::traverse(Node *node, int const &data) {

  bool restart = false;
  while (true) {

    Node *curr = node;
    int field = nextField(curr, data);

    // traverse
    while (curr->get(field)!=nullptr) {

      curr = curr->get(field);

      field = nextField(curr, data);

      // We have found node
      if (field==HERE) {

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
    bool goLeft = (data < curr->getData() ? true : false);
    Node *snapShot = (goLeft? curr->leftSnap.load() : curr->rightSnap.load());
    if (curr->mark || (goLeft && (data < snapShot->getData()))||
        (!goLeft &&(data > snapShot->getData()))) {
      curr->lock.unlock();
      continue;
    }

    return curr;
  }
}

void BinarySearchTree::insert(int const &data) {

  // Otherwise we traverse
  while (true) {
    Node *curr = traverse(root, data);

    // We have a duplicate
    if (curr->getData()== data) {
      curr->lock.unlock();
      return;
    }
    
    // No longer a leaf node
    if (
      (data > curr->getData() && curr->getRight()!=nullptr) ||
      (data < curr->getData() && curr->getLeft()!=nullptr)) {

      curr->lock.unlock();
      continue;
    }

    Node *newNode = new Node(data);
    newNode->setParent(curr);

    // Copy snaps from parent
    bool parentIsLarger = data < curr->getData();
    if (parentIsLarger) {
      newNode->leftSnap = curr->leftSnap.load();
      newNode->rightSnap = curr;
    } else {
      newNode->leftSnap = curr;
      newNode->rightSnap = curr->rightSnap.load();
    }

    // Update Snaps
    updateSnaps(newNode);
    
    // Add to path an update snaps
    if (parentIsLarger) {
      curr->setLeft(newNode);
    } else {
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

void BinarySearchTree::remove(int const &data) {

  Node *maxSnapNode;
  Node *minSnapNode;

  while (true) {
    Node *curr = traverse(root, data);

    // Already checked snapshots so return if current
    // node is not one to be deleted
    if (curr->getData()!= data) {
      return;
    }

    // Lock all nodes
    Node *parent = curr->getParent();
    if (!parent->lock.try_lock()) {
      continue;
    }

    bool parentIsLarger = (parent->getData() > data ? true : false);

    /*  A leaf node */
    Node *leftChild = curr->getLeft();
    Node *rightChild = curr->getRight();
    if (leftChild== nullptr && rightChild == nullptr) {
      maxSnapNode = curr->rightSnap.load();
      minSnapNode = curr->leftSnap.load();
      if (parentIsLarger) {
        parent->setLeft(nullptr);
        parent->leftSnap = minSnapNode;
        minSnapNode->rightSnap = parent;
      } else {
        parent->setRight(nullptr);
        parent->rightSnap = maxSnapNode;
        maxSnapNode->leftSnap = parent;
      }
      return;
    }

    if (leftChild !=nullptr)
      leftChild->lock.lock();
    if (rightChild !=nullptr)
      rightChild->lock.lock();

    /* A node with at most 1 child */
    if (leftChild==nullptr || rightChild==nullptr) {
      maxSnapNode = curr->rightSnap.load();
      minSnapNode = curr->leftSnap.load();
      bool hasRightChild = leftChild == nullptr;
      Node *snapshot =  (hasRightChild) ? maxSnapNode : minSnapNode;
      Node *currChild = (hasRightChild) ? rightChild : leftChild;
      
      // if the snapshot of curr is not its child then lock
      // that node as its path can be altered
      bool lockedSnap = false;
      if (snapshot!=currChild) {
        snapshot->lock.lock();
        lockedSnap = true;
      }

      // if the snapshot has changed restart
      if ((hasRightChild && snapshot!=curr->rightSnap.load()) || 
          (!hasRightChild && snapshot!=curr->leftSnap.load())) {
        if (lockedSnap) snapshot->lock.unlock(); // Released in reverse locking order
        currChild->lock.unlock();
        curr->lock.unlock();
        parent->lock.unlock();
        continue;
      }

      curr->mark = true;
      currChild->setParent(parent);

      if (parent->getLeft()==curr) {
        parent->setLeft(currChild);
      } else {
        parent->setRight(currChild);
      }

      // TODO: Update Snaps
      minSnapNode->rightSnap = maxSnapNode;
      maxSnapNode->leftSnap = minSnapNode;

      // TODO: Unlock all
      if (lockedSnap) snapshot->lock.unlock(); // Released in reverse locking order
      currChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();

      // delete curr;
      return;
    }

    maxSnapNode = curr->rightSnap.load();
    minSnapNode = curr->leftSnap.load();
    minSnapNode->lock.lock();

    if (minSnapNode->rightSnap!=curr || minSnapNode->mark) {
      minSnapNode->lock.unlock();
      rightChild->lock.unlock();
      leftChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();
      continue;
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
      minSnapNode->rightSnap = maxSnapNode;
      maxSnapNode->leftSnap = minSnapNode;
      // TODO: Unlock all
      rightChild->lock.unlock();
      leftChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();
      
      return;
    }

    Node *successor = traverse(curr->getRight(), data);
    if (maxSnapNode!=successor) {
      minSnapNode->lock.unlock();
      successor->lock.unlock();
      rightChild->lock.unlock();
      leftChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();
      continue;
    }

    Node *successorParent = successor->getParent();
    if (!successorParent->lock.try_lock()) {
      minSnapNode->lock.unlock();
      successor->lock.unlock();
      rightChild->lock.unlock();
      leftChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();
      continue;
    }
    
    Node *successorRightSnapshot = successor->rightSnap;
    bool lockedSuccessorSnap = false;
    Node *successorRightChild = successor->getRight();
    if (successorRightChild!=nullptr)  {
      successorRightChild->lock.lock();
      if (successorRightChild->getLeft()!=nullptr) {
        successorRightSnapshot->lock.lock();
        lockedSuccessorSnap = true;
      }
      if (successorRightSnapshot->leftSnap!=successor||
          successorRightSnapshot->mark) {
        if (lockedSuccessorSnap) successorRightSnapshot->lock.unlock();
        minSnapNode->lock.unlock();
        successor->lock.unlock();
        rightChild->lock.unlock();
        leftChild->lock.unlock();
        curr->lock.unlock();
        parent->lock.unlock();
      }
    }
    // Mark the node
    curr->mark = true;

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
    successor->rightSnap = successorRightSnapshot;
    successorRightSnapshot->leftSnap = successor;

    successor->leftSnap = minSnapNode;
    minSnapNode->rightSnap = successor;

    // Unlock All
    if (lockedSuccessorSnap) successorRightSnapshot->lock.unlock();
    minSnapNode->lock.unlock();
    successor->lock.unlock();
    rightChild->lock.unlock();
    leftChild->lock.unlock();
    curr->lock.unlock();
    parent->lock.unlock();

    // delete curr;
    return;
  }
}

bool BinarySearchTree::contains(int const &data) {
  bool restart = false;
  while (true) {

    Node *curr = root;
    int field = nextField(curr, data);

    // traverse
    while (curr->get(field)!=nullptr) {

      curr = curr->get(field);

      field = nextField(curr, data);

      // We have found node
      if (field==HERE) {

        // If marked then break from first while loop and restart
        if (curr->mark) {
          restart = true;
          break;
        }

        // Only executed if curr is not marked
        return true;
      }
    }

    if (restart == true) {
      restart = false;
      continue;
    }

    // grab snapshot
    // check if restart is needed
    bool goLeft = (data < curr->getData() ? true : false);
    Node *snapShot = (goLeft? curr->leftSnap : curr->rightSnap);
    if (curr->mark || (goLeft && (data < snapShot->getData()))||
        (!goLeft &&(data > snapShot->getData()))) {
      continue;
    }

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

void printSnaps(BinarySearchTree bst) {
  Node *start = bst.root;
  std::queue<Node *> q;
  q.push(start);

  while(!q.empty()) {

    Node *curr = q.front();
    q.pop();
    std::cout<<curr->getData()<<": ("<<curr->leftSnap.load()->getData()<<",";
    std::cout<<curr->rightSnap.load()->getData()<<")\n";
    if (curr->getLeft()!=nullptr) q.push(curr->getLeft());
    if (curr->getRight()!=nullptr) q.push(curr->getRight());
  }
}

std::vector<Node *> init_list(int num) {
  std::vector<Node *> vector;
  for (int i=0; i<num; i++) {
    Node *n = new Node(i);
    vector.push_back(n);
  }
  return vector;
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
int main() {

  BinarySearchTree bst;

  bst.insert(7);
  bst.insert(3);
  bst.insert(12);
  bst.insert(9);
  bst.insert(1);
  bst.insert(6);
  bst.insert(14);
  bst.insert(10);
  bst.insert(11);

  preOrderTraversal(bst);
  inOrderTraversal(bst);
  printSnaps(bst);
  printf("\n");

  printf("Remove 11\n");
  bst.remove(11);
  preOrderTraversal(bst);
  inOrderTraversal(bst);
  printSnaps(bst);
  printf("\n");

  printf("Remove 10\n");
  bst.insert(11);
  bst.remove(10);
  preOrderTraversal(bst);
  inOrderTraversal(bst);
  printSnaps(bst);
  printf("\n");

  printf("Remove 12\n");
  bst.remove(12);
  preOrderTraversal(bst);
  inOrderTraversal(bst);
  printSnaps(bst);
  printf("\n");

  printf("Remove 7\n");
  bst.remove(7);
  preOrderTraversal(bst);
  inOrderTraversal(bst);
  printSnaps(bst);
  printf("\n");
  return 0;
}


// int main(int argc, char **argv) {
  
//   if (argc!=6) {
//     std::cout<<"Please enter correct number of arguments!"<<std::endl;
//     return -1;
//   }
  
//   int n_threads = std::atoi(argv[1]);
//   std::thread threads[n_threads];
//   int total = std::atoi(argv[2])/n_threads;
//   int push = std::atoi(argv[3]);
//   int pop = std::atoi(argv[4]);
//   int size = std::atoi(argv[5]);

//   // List order of operations
//   std::vector<int> ops = init_ops(total, total*push/100, total*pop/100, total*size/100);
//   // lists nodes to work with
//   std::vector<node *> toGoThrough = init_list(total*n_threads);
//   DescriptorStack s = DescriptorStack();
//   // where the magic happens
//   for (int j=0; j<10; j++) {
//     std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

//     for (int i=0; i<n_threads; i++) {
//       threads[i] = std::thread(routine_4, std::ref(s), std::ref(toGoThrough), i+1, n_threads, std::ref(ops));  
//     }
//     for (int i=0; i<n_threads; i++) {
//       threads[i].join();
//     }

//     std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double, std::milli> time_span = t2 - t1;

//     // print_stack(s);
//     std::cout<<time_span.count()<<std::endl;
//     // std::cout<<s.size()<<"\n\n";
//   }
// }