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

int BinarySearchTree::nextField(Node *node, int const &data) {

  // c1(node, data) = L
  if (node == nullptr) std::cout<<"yes"<<std::endl;
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
				curr->lock.lock();
        // std::cout<<"locked node"<<std::endl;
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
		bool goLeft = (data < curr->getData() ? true : false);
		Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
		if (curr->mark || 
      (goLeft && (data <= snapShot->getData())) ||
			(!goLeft && (data >= snapShot->getData()))) {
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
    Node *snapshot = (parentIsLarger ? curr->leftSnap.load() : curr->rightSnap.load());

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

    // Unlock
    curr->lock.unlock();

    // Perform AVL rotations if applicable
    if (isAvl) {
      rebalance(curr);
    }
    
  }
}

void BinarySearchTree::remove(int const &data, int &thread_id) {

  Node *maxSnapNode;
  Node *minSnapNode;
  int id = thread_id;
  // printf("Thread %d : \n", id);
  while (true) {
    Node *curr = traverse(root, data);

    // Already checked snapshots so return if current
    // node is not one to be deleted
    if (curr->getData()!= data) {
      // std::cout<<"no node"<<std::endl;
      curr->lock.unlock();
      return;
    }
    
    // printf("%d: 0\n", id);
    // Lock all nodes
    Node *parent = curr->getParent();
    if (!parent->lock.try_lock()) {
      curr->lock.unlock();
      continue;
    }

		//Some other thread has gone and changed things around
		//Got to check if we already got removed otherwise unlock restart
		if (parent != curr->getParent()) {
      curr->lock.unlock();
			parent->lock.unlock();
			if (curr->mark) {
				return;
			}
			continue;
		}

    // printf("%d: 1\n", id);
    /*  A leaf node */
    Node *leftChild = curr->getLeft();
    Node *rightChild = curr->getRight();
    bool parentIsLarger = (parent->getData() > data ? true : false);
    
    if (leftChild== nullptr && rightChild == nullptr) {

      maxSnapNode = curr->rightSnap.load();
      minSnapNode = curr->leftSnap.load();
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

      curr->lock.unlock();
      parent->lock.unlock();

      if (isAvl) rebalance(parent);
      return;
    }

    // printf("%d: 2\n", id);
    if (leftChild !=nullptr) {
      // std::cout<<id<<": leftChild :"<<leftChild->getData()<<"\n";
      leftChild->lock.lock();
    }
    // printf("%d: 2a\n", id);
    if (rightChild !=nullptr) {
      // std::cout<<id<<": RightChild :"<<rightChild->getData()<<"\n";
      rightChild->lock.lock();
    }

    // printf("%d: 3\n", id);
    /* A node with at most 1 child */
    if (leftChild==nullptr || rightChild==nullptr) {

      bool hasRightChild = leftChild == nullptr;
      Node *currChild = (hasRightChild) ? rightChild : leftChild;

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

      // if the snapshot has changed restart
      if ((hasRightChild && snapshot->leftSnap.load()!=curr) || 
          (!hasRightChild && snapshot->rightSnap.load()!=curr) ||
					snapshot->mark) {

        if (lockedSnap) snapshot->lock.unlock(); // Released in reverse locking order
        currChild->lock.unlock();
        curr->lock.unlock();
        parent->lock.unlock();
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

      // TODO: Unlock all
      if (lockedSnap) snapshot->lock.unlock(); // Released in reverse locking order
      currChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();

      if (isAvl) rebalance(parent);
      return;
    }

    /* Hard Cases */
    minSnapNode = curr->leftSnap.load();
    maxSnapNode = curr->rightSnap.load();

    // printf("%d: 4\n", id);
    bool lockedPred = false;
    if (minSnapNode != leftChild) {
      minSnapNode->lock.lock();
      lockedPred = true;
    } 
    if (minSnapNode->rightSnap!=curr || minSnapNode->mark) {
      if (lockedPred) minSnapNode->lock.unlock();
      rightChild->lock.unlock();
      leftChild->lock.unlock();
      curr->lock.unlock();
      parent->lock.unlock();
      continue;
    }

    // std::cout<<id<<": predecessor :"<<minSnapNode->getData()<<"\n";
    // printf("%d: 5\n", id);
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

    // printf("%d: 6\n", id);
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

    // printf("%d: 7\n", id);
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

    // std::cout<<id<<": succParent :"<<succParent->getData()<<"\n";
    // printf("%d: 8\n", id);
    bool lockedSuccRightSnap = false;
    bool lockedSuccRightChild = false;
    
    Node *succRightChild = succ->getRight();
    Node *succRightSnapshot = succ->rightSnap.load();

    if (succRightChild!=nullptr)  {

      // printf("%d: 9\n", id);
      succRightChild->lock.lock();
      lockedSuccRightChild = true;

      // printf("%d: 10\n", id);
      succRightSnapshot = succ->rightSnap.load();
      if (succRightSnapshot!=succRightChild) {
        succRightSnapshot->lock.lock();
        lockedSuccRightSnap = true;
      }

      // printf("%d: 11\n", id);
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
    // printf("%d: 12\n", id);
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
      rebalanceSynchronized(succ);
      rebalanceSynchronized(succParent);
    } 

    return;
  }
}

bool BinarySearchTree::contains(int const &data) {
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
    Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
    if (curr->mark || 
      (goLeft && (data <= snapShot->getData())) ||
      (!goLeft && (data >= snapShot->getData()))) {
      curr->lock.unlock();
      continue;
    }

    return false;
  }
}

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
 * @return
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
      child->lock.unlock();

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

void routine_1(BinarySearchTree &bst, int id, int n_threads, 
              std::vector<int> keys) {

  int count = 0;
  for (int i=1*id; i<keys.size(); i+=n_threads) {
    // printf("Thread %d: insert #%d\n", id, count);
    bst.insert(i);
    // count++;
  }
  // std::cout<<"Check "<<check(bst)<<"!"<<std::endl;
  // bst.remove(-10, id);
  int increment = n_threads;
  for (int i=1*id; i<keys.size(); i++) {
    // std::cout<<i<<std::endl;
    // bst.remove(keys.at(i), id);
    // printf("Thread %d: remove #%d\n", id, count);
    // count++;
  }
}


int main(int argc, char **argv) {
  
  if (argc!=2) {
    std::cout<<"Please enter correct number of arguments!"<<std::endl;
    return -1;
  }
  
  int n_threads = std::atoi(argv[1]);
  std::thread threads[n_threads];
  
  std::mutex lock;

  std::vector<int> keys = init_list_ints(20);
  BinarySearchTree bst = BinarySearchTree(true);
  for (int i=0; i<n_threads; i++) {
    threads[i] = std::thread(routine_1, std::ref(bst), i, n_threads, std::ref(keys));  
  }
  for (int i=0; i<n_threads; i++) {
    threads[i].join();
  }
  printf("Preorder :");
  preOrderTraversal(bst);
  printf("Inorder :");
  inOrderTraversal(bst);
  printf("Snaps: \n");
  printSnaps(bst);
  printf("\n");
}