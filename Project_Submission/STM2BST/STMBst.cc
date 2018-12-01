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
  delete root;
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

/**
 * Traverse to node and lock it. If tree contains node, we attempt to
 * lock the node. Check if marked. If not, the last node is the one to be 
 * inserted. Check if the key(data) is in the snapshos of the node. If not
 * restart the traversal.
 * 
 * @param  node Starting node
 * @param  data key value to search for
 * @return      The last node in the traversal which is now locked.
 */
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


/**
 * BinarySearchTree::insert Insert new node into tree. If tree contains node
 * no node is inserted
 *
 * 
 * @param data key to be inserted into tree
 */
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

    if (isAvl) rebalance(curr);
    return;
  }
}


/**
 * BinarySearchTree::remove Removes node from tree. If node is not present then the
 * call returns. 
 * @param data The key to be removed from the tree
 */
void STMBst::remove(int const &data) {

  Node *maxSnapNode;
  Node *minSnapNode;

  Node *toBalance1 = nullptr;
  Node *toBalance2 = nullptr;

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

      
      Node *leftChild = curr->getLeft();
      Node *rightChild = curr->getRight();
      bool parentIsLarger = (parent->getData() > data ? true : false);

      /*  A leaf node */
      if (leftChild == nullptr && rightChild == nullptr) {

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

        toBalance1 = parent;

         /* A node with at most 1 child */
      } else if (leftChild == nullptr || rightChild == nullptr) {

        bool hasRightChild = leftChild == nullptr;
        Node *currChild = (hasRightChild) ? rightChild : leftChild;

        //Another possible section
        minSnapNode = curr->leftSnap;
        maxSnapNode = curr->rightSnap;

        Node *snapshot = (hasRightChild) ? maxSnapNode : minSnapNode;

        // if the snapshot has changed restart
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

        toBalance1 = parent;

        /* Hard Cases */ 
      } else {
        
        minSnapNode = curr->leftSnap;
        maxSnapNode = curr->rightSnap;

        if (minSnapNode->rightSnap != curr || minSnapNode->mark) {
          continue;
        }

        /* Node with where the right child's left node is null */
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

          toBalance1 = rightChild;
        } else {
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
          
          succ->leftSnap = minSnapNode;
          minSnapNode->rightSnap = succ; 

          toBalance1 = succ;
          toBalance2 = succParent;
        }
      }
    }

    if (isAvl) {
      rebalance(toBalance1);
      if (toBalance2!=nullptr) rebalance(toBalance2);
    }
    return;
  }
}


/**
 * BinarySearchTree::contains Returns true if tree contains node and false otherwise
 * @param  data key to search for
 * @return      A boolean value
 */
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
    __transaction_atomic{
      Node *snapShot = (goLeft? curr->leftSnap : curr->rightSnap);
      if (curr->mark || (goLeft && (data < snapShot->getData()))||
          (!goLeft &&(data > snapShot->getData()))) {
        continue;
      }
    }
    return false;
  }
}

// Rotates node to the left. Child becomes nodes parent.
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

//Rotates node to the right. Child becomes nodes parent
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

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 */
void STMBst::rebalance(Node *node) {

  if (node==root) {
    return;
  }

  Node *parent = node->getParent();
  Node *curr = node;
  Node *child;
  Node *grandChild;

  // Condition for traversing to next node
  int cond = -1;
  while(curr!=root) {

    __transaction_atomic {

      if (curr->getParent()!=parent) {
        if (curr->mark) {
          return;
        }
        parent = curr->getParent();
        continue;
      }
      if (curr->mark) {
        return;
      }

      Node *left = curr->getLeft();
      Node *right= curr->getRight();

      int leftHeight = height(left);
      int rightHeight = height(right);

      int currHeight = std::max(leftHeight, rightHeight) + 1;
      int prevHeight = curr->getHeight();

      int bf = leftHeight - rightHeight;
      if (currHeight != prevHeight) {
        curr->setHeight(currHeight);
      } else if (bf <= 1) {
        return;
      }

      // The curr's right subtree is too tall
      if (bf < MINBF) {
        child = right;

        Node *childLeft = child->getLeft();
        Node *childRight = child->getRight();

        int childLeftHeight = height(childLeft);
        int childRightHeight = height(childRight);

        int childBf = childLeftHeight - childRightHeight;

        grandChild = childLeft;

        if (childBf > 0) {
          rotateRight(grandChild, child, curr);
          rotateLeft(grandChild, curr, parent);
          cond = 1;

        } else {
          rotateLeft(child, curr, parent);
          cond = 2;
        }

        // The curr's left subtree is too tall
      } else if (bf > MAXBF) {
        child = left;

        Node *childLeft = child->getLeft();
        Node *childRight = child->getRight();

        int childLeftHeight = height(childLeft);
        int childRightHeight = height(childRight);

        int childBf = childLeftHeight - childRightHeight;

        grandChild = childRight;

        if (childBf < 0) {
          rotateLeft(grandChild, child, curr);
          rotateRight(grandChild, curr, parent);
          cond = 1;
        } else {
          rotateRight(child, curr, parent);
          cond = 2;
        }

      } else {
        // Traverse back up tree
        cond = 3;
      }
    } // transatomic

    // where to traverse next
    switch(cond) {
      case 1: {
        curr = grandChild;
        break;
      }
      case 2: {
        curr = child;
        break;
      }
      case 3: {
        curr = parent;
        parent = curr->getParent();
        break;
      }
      default : {
        // impossible
        break;
      }
    } // switch
  }// while loop
}


/**
 *
 * UTILITY FUNCTIONS
 * 
 */

void inOrderTraversal(STMBst &bst) {

  std::stack<Node*> stack;

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

void routine_4(STMBst &bst, int id, int n_threads, std::vector<int> keys, std::vector<int> ops) {

  int count = 0;
  int add = id;
  int rem = id;
  int cont = id;
  for (int i=0; i<ops.size(); i++) {
    if (ops.at(i)==0) {
      bst.insert(keys.at(add));
      add+=n_threads;
    } else if (ops.at(i) == 1) {
      bst.remove(keys.at(rem));
      rem+=n_threads;
    } else {
      bst.contains(keys.at(cont));
      rem+=n_threads;
    }
  }
}

std::vector<int> init_ops(int max, int add, int rem, int cont) {
  std::vector<int> ops;
  for (int i=0; i<add; i++) {
    ops.push_back(0);
  }
  for (int i=0; i<rem; i++) {
    ops.push_back(1);
  }
  for (int i=0; i<cont; i++) {
    ops.push_back(2);
  }
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(ops), std::end(ops), rng);
  return ops;
}

class NodeDepth {
public:
  Node * node;
  int depth;
  NodeDepth(Node *n, int d) {
    node=n;
    depth = d;
  }
};

void printTreeDepth(STMBst bst) {
  Node *start = bst.root;
  std::queue<NodeDepth *> q;
  q.push(new NodeDepth(start, 0));

  int prevDepth = 0;
  while(!q.empty()) {

    NodeDepth *curr = q.front();
    Node *currNode = curr->node;
    int currDepth = curr->depth;
    q.pop();
    delete curr;

    if (currDepth!=prevDepth) {
      std::cout<<"\n";
      prevDepth = currDepth;
    }
    if (currNode!=nullptr) {
      std::cout<<currNode->getData()<<" ";
    } else {
      std::cout<<"- ";
    }
    if (currNode!=nullptr) {
      q.push(new NodeDepth(currNode->getLeft(), currDepth + 1));
      q.push(new NodeDepth(currNode->getRight(), currDepth + 1));
    } 
  }
}

STMBst *init_BST(int numberOfNodes, bool AVL, std::random_device &rd) {

  STMBst  *bst = new STMBst(AVL);
  // std::cout<<numberOfNodes<<std::endl;

  int min = -1*numberOfNodes;
  std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
  std::uniform_int_distribution<int> uni(min,0); // guaranteed unbiased
  auto random_integer = uni(rng);

  int start = random_integer;
  int end = 2*numberOfNodes + start;

  std::vector<int> nodevals;
  int count = 0;
  for (int i=start; i<end; i+=2) {
    // std::cout<<"push\n";
    nodevals.push_back(i);
    count++;
  }
  std::shuffle(std::begin(nodevals), std::end(nodevals), rng);

  for (int i=0; i<nodevals.size(); i++) {
    bst->insert(nodevals.at(i));
  }

  return bst;
}


int main(int argc, char **argv) {
  
  if (argc!=6) {
    std::cout<<"Please enter correct number of arguments!"<<std::endl;
    return -1;
  }

  // Initialize device
  std::random_device rd;
  // int [] numThreads= {1, 2, 4, 8, 16, 32};
  const std::vector<int> numThreads= {1, 2, 4, 8};
  for (int t = 0; t<numThreads.size(); t++ ) {

    int n_threads = numThreads.at(t);
    std::thread threads[n_threads];

    bool avlProp = std::atoi(argv[1]);

    int total = std::atoi(argv[2])/n_threads;
    int add = std::atoi(argv[3]);
    int rem = std::atoi(argv[4]);
    int cont = std::atoi(argv[5]);

    std::vector<int> keys = init_list_ints(total*n_threads);
    std::vector<int> ops = init_ops(total, total*add/100, total*rem/100, total*cont/100);

    double avg = 0;
    int runs = 10;
    for (int run=0; run<runs; run++) {
      STMBst *bst = init_BST(total*n_threads*add/100, avlProp, rd);
      std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      for (int i=0; i<n_threads; i++) {
        threads[i] = std::thread(routine_4, std::ref(*bst), i, n_threads, std::ref(keys), std::ref(ops));  
      }
      for (int i=0; i<n_threads; i++) {
        threads[i].join();
      }
      std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> time_span = t2 - t1;

      // std::cout<<time_span.count()<<std::endl;
      avg += time_span.count();
      delete bst;
    }
    // std::cout<<n_threads<<" ";
    std::cout<<avg/((double) runs)<<std::endl;
  }
}