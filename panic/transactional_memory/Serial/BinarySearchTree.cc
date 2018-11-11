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
  __transaction_atomic{
	  if (field == LEFT){
			(parent->leftSnap)->rightSnap = node;
			parent->leftSnap = node;
	  } else {
			(parent->rightSnap)->leftSnap = node;
			parent->rightSnap = node;
	  }
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
        //curr->lock.lock();
        if (curr->mark) {
          restart = true;
          //curr->lock.unlock();
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

    //curr->lock.lock();

    // grab snapshot
    // check if restart is needed
    bool goLeft = (data < curr->getData() ? true : false);
		 
		__transaction_atomic{
			Node* snapShot = (goLeft? curr->leftSnap : curr->rightSnap);
			
			if (curr->mark || (goLeft && (data < snapShot->getData())) ||
					(!goLeft &&(data > snapShot->getData()))) {
				//curr->lock.unlock();
				continue;
			}
		}
    return curr;
  }
}

void BinarySearchTree::insert(int const &data) {

  // Otherwise we traverse
  while (true) {
    Node *curr = traverse(root, data);
		__transaction_atomic {

			// We have a duplicate
			if (curr->getData()== data) {
				//curr->lock.unlock();
				return;
			}
    
			// No longer a leaf node
			if (
				(data > curr->getData() && curr->getRight()!=nullptr) ||
				(data < curr->getData() && curr->getLeft()!=nullptr)) {

				//curr->lock.unlock();
				continue;
			}

			Node *newNode = new Node(data);
			newNode->setParent(curr);

			// Copy snaps from parent
			bool parentIsLarger = data < curr->getData();
			__transaction_atomic{
				if (parentIsLarger) {
					newNode->leftSnap = curr->leftSnap;
					newNode->rightSnap = curr;
				} else {
					newNode->leftSnap = curr;
					newNode->rightSnap = curr->rightSnap;
				}
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
			//if (isAvl){
			// Update heights
			//	updateHeights(curr);
			//	rebalance(curr);
			//}
		}
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
    std::cout<<curr->getData()<<": ("<<curr->leftSnap->getData()<<",";
    std::cout<<curr->rightSnap->getData()<<")\n";
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

void doInserts(BinarySearchTree bst, int index){
	int range;
	int numberOfTestCases = 1000;
	range = numberOfTestCases * (index + 1);
	for (int i = numberOfTestCases * index; i < range; ++i) {
		bst.insert(i);
	}
}

void doContains(BinarySearchTree bst) {
	for (int i = 0; i < 10000; ++i){
		int randomNumToFind = std::rand() % 1000;	
		
		//if (bst.contains(randomNumToFind)) {
		//	__transaction_relaxed{
		//		std::cout << "Found number : " << randomNumToFind << std::endl;
		//	}
		//}
		//else {
		//	__transaction_relaxed{
		//		std::cout << randomNumToFind << " not found!" << std::endl;
		//	}
		//}	
	}
}



int main() {

	BinarySearchTree bst;
	//*bst = new BinarySearchTree();

	std::thread t1(doInserts, (BinarySearchTree)bst, 0);
	std::thread t2(doInserts, (BinarySearchTree)bst, 1);
	std::thread t3(doInserts, (BinarySearchTree)bst, 2);
	std::thread t4(doInserts, (BinarySearchTree)bst, 3);
	std::thread t5(doInserts, (BinarySearchTree)bst, 4);
	std::thread t6(doInserts, (BinarySearchTree)bst, 5);
	std::thread t7(doInserts, (BinarySearchTree)bst, 6);
	std::thread t8(doInserts, (BinarySearchTree)bst, 7);
	std::thread t9(doInserts, (BinarySearchTree)bst, 8);
	
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();
	t8.join();
	t9.join();

	preOrderTraversal(bst);
	inOrderTraversal(bst);
	//printSnaps(bst);
	printf("\n");

	return 0;
}