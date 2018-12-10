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
#include <chrono>
#include <cstdlib>
#include <ctime>

// Constants
const int MAXBF = 1; // Max Balance Factor
const int MINBF = -1; // Min Balance Factor
const int iMin = std::numeric_limits<int>::min(); // Min Integer
const int iMax = std::numeric_limits<int>::max(); // Max Integer
// For traversals
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

  this->root = maxSentinel;
}

BinarySearchTree::~BinarySearchTree() {
  delete minSentinel;
}

Node *BinarySearchTree::getRoot() {
  return root;
}

Node *BinarySearchTree::getMinSentinel() {
	return minSentinel;
}

Node *BinarySearchTree::getMaxSentinel() {
	return maxSentinel;
}

int BinarySearchTree::nextField(Node *node, int const &key) {

  // c1(node, key) = L
  if (key<node->getKey()) return LEFT;

  // c2(node, key) = R
  if (key>node->getKey()) return RIGHT;

  // c3(node, key) = null;
  return HERE;
}


/**
 * Traverse to node and lock it. If tree contains node, we attempt to
 * lock the node. Check if marked. If not, the last node is the one to be 
 * inserted. Check if the key(key) is in the snapshos of the node. If not
 * restart the traversal.
 * 
 * @param  node Starting node
 * @param  key key value to search for
 * @return      The last node in the traversal which is now locked.
 */
Node *BinarySearchTree::traverse(Node *node, int const &key) {
	bool restart = false;
	while (true) {

		Node *curr = root;
		int field = nextField(curr, key);

		// traverse
    Node *next = curr->get(field);
		while (next != nullptr) {

			curr = next;

			field = nextField(curr, key);

			// We have found node
			if (field == HERE) {
				curr->lock.lock();
				// If marked then break from first while loop and restart
				if (curr->mark) {
					curr->lock.unlock();
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
		curr->lock.lock();
		// grab snapshot
		// check if restart is needed
		bool goLeft = (key < curr->getKey() ? true : false);
		Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
		if (curr->mark || 
      		(goLeft && (key <= snapShot->getKey())) ||
			(!goLeft && (key >= snapShot->getKey()))) {
			curr->lock.unlock();
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
 * @param key key to be inserted into tree
 */
void BinarySearchTree::insert(int const &key) {

  // Continue to attempt insertion
  while (true) {

    // traverse and lock node
    Node *curr = traverse(root, key);
  
    // We have a duplicate
    if (curr->getKey()== key) {
      curr->lock.unlock();
      return;
    }
    
    
    // No longer a leaf node
    if (
      (key > curr->getKey() && curr->getRight()!=nullptr) ||
      (key < curr->getKey() && curr->getLeft()!=nullptr)) {

      curr->lock.unlock();
      continue;
    }
    
    // Insert node and update parent
    Node *newNode = new Node(key);
    newNode->setParent(curr);

    // Copy snaps from parent
    bool parentIsLarger = key < curr->getKey();
    Node *snapshot = (parentIsLarger ? curr->leftSnap.load() : curr->rightSnap.load());


    // If parent is larger, set left pointer to new node
    // and update snaps
    if (parentIsLarger) {
      newNode->leftSnap = snapshot;
      newNode->rightSnap = curr;

      snapshot->rightSnap = newNode;
      curr->leftSnap = newNode;

      curr->setLeft(newNode);

    // Otherwise set right pointer and update snaps
    } else {
      newNode->leftSnap = curr;
      newNode->rightSnap = snapshot;

      snapshot->leftSnap = newNode;
      curr->rightSnap = newNode;

      curr->setRight(newNode);
    }

    // Unlock
    curr->lock.unlock();

    // Perform AVL rotations if applicable
    if (isAvl) {
      rebalance(curr);
    }
    return;
  }
}

/**
 * BinarySearchTree::remove Removes node from tree. If node is not present then the
 * call returns. 
 * @param key The key to be removed from the tree
 */
void BinarySearchTree::remove(int const &key) {

  Node *maxSnapNode;
  Node *minSnapNode;
  
  // Continually attempt removal until call is returned
  while (true) {

    // Grab node
    Node *curr = traverse(root, key);

    // Already checked snapshots so return if current
    // node is not one to be deleted
    if (curr->getKey()!= key) {
      curr->lock.unlock();
      return;
    }
    
    // Lock Parent
    Node *parent = curr->getParent();
    if (!parent->lock.try_lock()) {
      curr->lock.unlock();
      continue;
    }

		// Some other thread has gone and changed things around
		// Got to check if we already got removed otherwise unlock restart
		if (parent != curr->getParent()) {
      curr->lock.unlock();
			parent->lock.unlock();
			if (curr->mark) {
				return;
			}
			continue;
		}

    
    Node *leftChild = curr->getLeft();
    Node *rightChild = curr->getRight();
    bool parentIsLarger = (parent->getKey() > key ? true : false);
    
    /*  A leaf node */
    if (leftChild== nullptr && rightChild == nullptr) {

      // Grab snapshots
      maxSnapNode = curr->rightSnap.load();
      minSnapNode = curr->leftSnap.load();

      // Logical Removal
      curr->mark = true;

      // Update pointers and snapshots
      if (parentIsLarger) {
        parent->setLeft(nullptr);
        parent->leftSnap = minSnapNode;
        minSnapNode->rightSnap = parent;
      } else {
        parent->setRight(nullptr);
        parent->rightSnap = maxSnapNode;
        maxSnapNode->leftSnap = parent;
      }

      // Unlock all
      curr->lock.unlock();
      parent->lock.unlock();

      if (isAvl) rebalance(parent);
      return;
    }

    // Lock children
    if (leftChild !=nullptr)leftChild->lock.lock();
    if (rightChild !=nullptr) rightChild->lock.lock();

    /* A node with at most 1 child */
    if (leftChild==nullptr || rightChild==nullptr) {

      bool hasRightChild = leftChild == nullptr;
      Node *currChild = (hasRightChild) ? rightChild : leftChild;

      // Load snaps
      minSnapNode = curr->leftSnap.load();
      maxSnapNode = curr->rightSnap.load();

      Node *snapshot =  (hasRightChild) ? maxSnapNode : minSnapNode;
      
      
      // if the snapshot of curr is not its child then lock
      // that node as its path can be altered
      bool lockedSnap = false;
      if (snapshot!=currChild) {
        snapshot->lock.lock();
        lockedSnap = true;
      }

      // if the snapshot has changed unlock all and restart
      if ((hasRightChild && snapshot->leftSnap.load()!=curr) || 
          (!hasRightChild && snapshot->rightSnap.load()!=curr) ||
					snapshot->mark) {

        if (lockedSnap) snapshot->lock.unlock(); 
        currChild->lock.unlock();
        curr->lock.unlock();
        parent->lock.unlock();
        continue;
      }

      // Logical removal
      curr->mark = true;
      currChild = (hasRightChild) ? rightChild : leftChild;
      if (parent->getLeft()==curr) {
        parent->setLeft(currChild);
      } else {
        parent->setRight(currChild);
      }

      currChild->setParent(parent);

      // Update Snaps
      minSnapNode->rightSnap = maxSnapNode;
      maxSnapNode->leftSnap = minSnapNode;

      // Unlock all
      if (lockedSnap) snapshot->lock.unlock(); 
      currChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();

      if (isAvl) rebalance(parent);
      return;
    }

    /* Hard Cases */
    minSnapNode = curr->leftSnap.load();
    maxSnapNode = curr->rightSnap.load();

    
    // Lock if leftSnap is not the leftChild
    bool lockedPred = false;
    if (minSnapNode != leftChild) {
      minSnapNode->lock.lock();
      lockedPred = true;
    } 

    // Check if the LeftSnapshot's right snapshot is the node
    // to be removed
    if (minSnapNode->rightSnap!=curr || minSnapNode->mark) {
      if (lockedPred) minSnapNode->lock.unlock();
      rightChild->lock.unlock();
      leftChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();
      continue;
    }

    
    /* Node with where the right child's left node is null */
    if (rightChild->getLeft() == nullptr) {

      curr->mark = true;

      // Updated pointers
      rightChild->setLeft(leftChild);
      leftChild->setParent(rightChild);
      rightChild->setParent(parent);

      if (parent->getLeft()==curr) {
        parent->setLeft(rightChild);
      } else {
        parent->setRight(rightChild);
      }

      // Update snaps
      minSnapNode->rightSnap = maxSnapNode;
      maxSnapNode->leftSnap = minSnapNode;

      // Unlock all
      if (lockedPred) minSnapNode->lock.unlock();
      rightChild->lock.unlock();
      leftChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();
      
      if (isAvl) rebalance(rightChild);

      return;
    }

    /* Hardest Case */

    Node *succ = maxSnapNode;
    Node *succParent = succ->getParent();

    // Successor's parent is no the right child
    bool lockedSuccParent = false;
    if (succParent!=rightChild) {
      succParent->lock.lock();

      if (maxSnapNode->getParent() != succParent || maxSnapNode->mark) {
        succParent->lock.unlock();
        if (lockedPred) minSnapNode->lock.unlock();
        rightChild->lock.unlock();
        leftChild->lock.unlock();
        curr->lock.unlock();
        parent->lock.unlock();
        continue;
      }

      lockedSuccParent = true;
    }

    // Lock successor
    succ->lock.lock();
    if (maxSnapNode->leftSnap.load()!=curr || maxSnapNode->mark) {
      succ->lock.unlock();
      if (lockedSuccParent) succParent->lock.unlock();
      if (lockedPred) minSnapNode->lock.unlock();
      rightChild->lock.unlock();
      leftChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();
      continue;
    }

    // Lock Right Child if successor has a right child
    // and rightSnap if it is not the right child
    bool lockedSuccRightSnap = false;
    bool lockedSuccRightChild = false;
    
    Node *succRightChild = succ->getRight();
    Node *succRightSnapshot = succ->rightSnap.load();

    if (succRightChild!=nullptr)  {

      succRightChild->lock.lock();
      lockedSuccRightChild = true;

      succRightSnapshot = succ->rightSnap.load();
      if (succRightSnapshot!=succRightChild) {
        succRightSnapshot->lock.lock();
        lockedSuccRightSnap = true;
      }

      // Check if it's left snap is still the successor
      if (succRightSnapshot->leftSnap.load()!=succ||succRightSnapshot->mark) {

        // Unlock all
        if (lockedSuccRightSnap) succRightSnapshot->lock.unlock();
        if (lockedSuccRightChild) succRightChild->lock.unlock();
        succ->lock.unlock();
        if (lockedSuccParent) succParent->lock.unlock();
        if (lockedPred) minSnapNode->lock.unlock();
        rightChild->lock.unlock();
        leftChild->lock.unlock();
        curr->lock.unlock();
        parent->lock.unlock();
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

    if (succRightChild!=nullptr)
      succRightChild->setParent(succParent);
    
    // Update Snaps
    succ->rightSnap = succRightSnapshot;
    succRightSnapshot->leftSnap = succ;

    succ->leftSnap = minSnapNode;
    minSnapNode->rightSnap = succ;

    // Unlock All
    if (lockedSuccRightSnap) succRightSnapshot->lock.unlock();
    if (lockedSuccRightChild) succRightChild->lock.unlock();
    succ->lock.unlock();
    if (lockedSuccParent) succParent->lock.unlock();
    if (lockedPred) minSnapNode->lock.unlock();
    rightChild->lock.unlock();
    leftChild->lock.unlock();
    curr->lock.unlock();
    parent->lock.unlock();

    if (isAvl) {
      rebalance(succ);
      rebalance(succParent);
    } 

    return;
  }
}


/**
 * BinarySearchTree::contains Returns true if tree contains node and false otherwise
 * @param  key key to search for
 * @return      A boolean value
 */
bool BinarySearchTree::contains(int const &key) {
  bool restart = false;
  while (true) {

    Node *curr = root;
    int field = nextField(curr, key);

    // traverse
    Node *next = curr->get(field);
    while (next != nullptr) {

      curr = next;

      field = nextField(curr, key);

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
    bool goLeft = (key < curr->getKey() ? true : false);
    Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
    if (curr->mark || 
      (goLeft && (key <= snapShot->getKey())) ||
      (!goLeft && (key >= snapShot->getKey()))) {
      curr->lock.unlock();
      continue;
    }

    return false;
  }
}

// Rotates node to the left. Child becomes nodes parent.
void BinarySearchTree::rotateLeft(Node *child, Node *node, Node *parent) {

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
void BinarySearchTree::rotateRight(Node *child, Node *node, Node *parent) {

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
int BinarySearchTree::height(Node *node) {
  return (node == nullptr) ? -1 : node->getHeight();
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 */
void BinarySearchTree::rebalance(Node *node) {

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
      std::cout<<(curr->getKey())<<" ";
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
    std::cout<<(curr->getKey())<<" ";
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

bool check (BinarySearchTree &bst) {
  Node *curr = bst.getMinSentinel();
  int currVal = curr->getKey();
  Node *last = bst.getMaxSentinel();
  while (curr!=last) {
    curr = curr->rightSnap;
    int nextVal = curr->getKey();
    if (nextVal <= currVal) return false;
    currVal = nextVal;
  }
  return true;
}

void routine_4(BinarySearchTree &bst, int id, int n_threads, std::vector<int> keys, std::vector<int> ops) {

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

void printTreeDepth(BinarySearchTree bst) {
  Node *start = bst.getRoot();
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
      std::cout<<currNode->getKey()<<" ";
    } else {
      std::cout<<"- ";
    }
    if (currNode!=nullptr) {
      q.push(new NodeDepth(currNode->getLeft(), currDepth + 1));
      q.push(new NodeDepth(currNode->getRight(), currDepth + 1));
    } 
  }
}

BinarySearchTree *init_BST(int numberOfNodes, bool AVL, std::random_device &rd) {

  BinarySearchTree *bst = new BinarySearchTree(AVL);

  int min = -1*numberOfNodes;
  std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
  std::uniform_int_distribution<int> uni(min,0); // guaranteed unbiased
  auto random_integer = uni(rng);

  int start = random_integer;
  int end = 2*numberOfNodes + start;

  std::vector<int> nodevals;
  int count = 0;
  for (int i=start; i<end; i+=2) {
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
    int runs = 1;
    for (int run=0; run<runs; run++) {
      BinarySearchTree *bst = init_BST(total*n_threads*add/100, avlProp, rd);
      std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      for (int i=0; i<n_threads; i++) {
      	threads[i] = std::thread(routine_4, std::ref(*bst), i, n_threads, std::ref(keys), std::ref(ops));  
      }
      for (int i=0; i<n_threads; i++) {
        threads[i].join();
      }
      std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> time_span = t2 - t1;

      avg += time_span.count();
      delete bst;
    }
    std::cout<<avg/((double) runs)<<std::endl;
  }
}