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
#include <cstdlib>
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
  if (data<node->getData()) return LEFT;

  // c2(node, data) = R
  if (data>node->getData()) return RIGHT;

  // c3(node, data) = null;
  return HERE;
}

void BinarySearchTree::updateSnaps(Node *node) {

  Node *parent = node->getParent();
  int field = nextField(parent, node->getData());
	//TODO: Check if this needs an atomic surrounding this!
	if (field == LEFT){
		(parent->leftSnap)->rightSnap = node;
		parent->leftSnap = node;
	} else {
		(parent->rightSnap)->leftSnap = node;
		parent->rightSnap = node;
	}
}

Node *BinarySearchTree::traverse(Node *node, int const &data) {

  bool restart = false;
  while (true) {

    Node *curr = node;
    int field = nextField(curr, data);
	Node *next;
    // traverse
	__transaction_atomic{
		next = curr->get(field);
	}
    while (true) {
			__transaction_atomic{
				if (next == nullptr) {
					break;
				}
			}
			curr = next;
			field = nextField(curr, data);

      // We have found node
      if (field==HERE) {

				__transaction_atomic{
					// If marked then break from first while loop and restart
					if (curr->mark) {
						restart = true;
						break;
					}
				}
        // Only executed if curr is not marked
        return curr;
      }
			__transaction_atomic{
				next = curr->get(field);
			}
    }

    if (restart == true) {
      restart = false;
      continue;
    }

    // grab snapshot
    // check if restart is needed
    bool goLeft = (data < curr->getData() ? true : false);
		Node* snapShot;
		//Purposely separated from the normal implementation for better performance
		__transaction_atomic{
			if (curr->mark) {
				continue;
			}
			snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
		}

		if ((goLeft && (data < snapShot->getData())) ||
				(!goLeft && (data > snapShot->getData()))) {
			continue;
		}
    return curr;
  }
}

void BinarySearchTree::insert(int const &data) {

  // Otherwise we traverse
  while (true) {
    Node *curr = traverse(root, data);
		//Acts as a lock for the node
		//Although it seems like it is currently locking the entire data structure down
			
		// We have a duplicate
		if (curr->getData()== data) {
			return;
		}
		__transaction_atomic{
			// No longer a leaf node
			if (
				(data > curr->getData() && curr->getRight()!=nullptr) ||
				(data < curr->getData() && curr->getLeft()!=nullptr)) {
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

		}
		// Perform AVL rotations if applicable
		if (isAvl){
			//Update heights
			updateHeights(curr);
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
		if (curr->getData() != data) {
			// std::cout<<"no node"<<std::endl;
			//curr->lock.unlock();
			return;
		}

		// printf("%d: 0\n", id);
		// Lock all nodes
		__transaction_atomic{
			Node *parent = curr->getParent();
			//if (!parent->lock.try_lock()) {
			//	curr->lock.unlock();
			//	continue;
			//}

			//Some other thread has gone and changed things around
			//Got to check if we already got removed otherwise unlock restart
			if (parent != curr->getParent()) {
				//curr->lock.unlock();
				//parent->lock.unlock();
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

			if (leftChild == nullptr && rightChild == nullptr) {

				//One possible atmoic segment
				maxSnapNode = curr->rightSnap;
				minSnapNode = curr->leftSnap;
				curr->mark = true;

				if (parentIsLarger) {
					parent->setLeft(nullptr);
					parent->leftSnap = minSnapNode;
					minSnapNode->rightSnap = parent;
				}
				else {
					parent->setRight(nullptr);
					parent->rightSnap = maxSnapNode;
					maxSnapNode->leftSnap = parent;
				}

				//curr->lock.unlock();
				//parent->lock.unlock();
				return;
			}

			// printf("%d: 2\n", id);
			//if (leftChild != nullptr) {
				// std::cout<<id<<": leftChild :"<<leftChild->getData()<<"\n";
				//leftChild->lock.lock();
			//}
			// printf("%d: 2a\n", id);
			//if (rightChild != nullptr) {
				// std::cout<<id<<": RightChild :"<<rightChild->getData()<<"\n";
				//rightChild->lock.lock();
			//}

			// printf("%d: 3\n", id);
			/* A node with at most 1 child */
			if (leftChild == nullptr || rightChild == nullptr) {

				bool hasRightChild = leftChild == nullptr;
				Node *currChild = (hasRightChild) ? rightChild : leftChild;

				//Another possible section
				minSnapNode = curr->leftSnap;
				maxSnapNode = curr->rightSnap;

				Node *snapshot = (hasRightChild) ? maxSnapNode : minSnapNode;


				// if the snapshot of curr is not its child then lock
				// that node as its path can be altered
				//bool lockedSnap = false;
				//if (snapshot != currChild) {
				//	snapshot->lock.lock();
				//	lockedSnap = true;
				//}

				// if the snapshot has changed restart
				// Atomic PlaceHolder
				if ((hasRightChild && snapshot->leftSnap != curr) ||
					(!hasRightChild && snapshot->rightSnap != curr) ||
					snapshot->mark) {

					//if (lockedSnap) snapshot->lock.unlock(); // Released in reverse locking order
					//currChild->lock.unlock();
					//curr->lock.unlock();
					//parent->lock.unlock();
					continue;
				}
				// Atomic PlaceHolder
				curr->mark = true;
				currChild = (hasRightChild) ? rightChild : leftChild;
				if (parent->getLeft() == curr) {
					parent->setLeft(currChild);
				}
				else {
					parent->setRight(currChild);
				}

				currChild->setParent(parent);

				// TODO: Update Snaps
				minSnapNode->rightSnap = maxSnapNode;
				maxSnapNode->leftSnap = minSnapNode;

				// TODO: Unlock all
				//if (lockedSnap) snapshot->lock.unlock(); // Released in reverse locking order
				//currChild->lock.unlock();
				//curr->lock.unlock();
				//parent->lock.unlock();

				// delete curr;
				return;
			}

			/* Hard Cases */
			minSnapNode = curr->leftSnap;
			maxSnapNode = curr->rightSnap;

			// printf("%d: 4\n", id);
			//bool lockedPred = false;
			//if (minSnapNode != leftChild) {
			//	minSnapNode->lock.lock();
			//	lockedPred = true;
			//}
			if (minSnapNode->rightSnap != curr || minSnapNode->mark) {
				//if (lockedPred) minSnapNode->lock.unlock();
				//rightChild->lock.unlock();
				//leftChild->lock.unlock();
				//curr->lock.unlock();
				//parent->lock.unlock();
				continue;
			}

			// std::cout<<id<<": predecessor :"<<minSnapNode->getData()<<"\n";
			// printf("%d: 5\n", id);
			/* Node with where the right child's left node is null */
			// Atomic PlaceHolder
			if (rightChild->getLeft() == nullptr) {
				curr->mark = true;

				rightChild->setLeft(leftChild);
				leftChild->setParent(rightChild);
				rightChild->setParent(parent);

				if (parent->getLeft() == curr) {
					parent->setLeft(rightChild);
				}
				else {
					parent->setRight(rightChild);
				}

				// TODO: update snaps
				minSnapNode->rightSnap = maxSnapNode;
				maxSnapNode->leftSnap = minSnapNode;
				// TODO: Unlock all
				//if (lockedPred) minSnapNode->lock.unlock();
				//rightChild->lock.unlock();
				//leftChild->lock.unlock();
				//curr->lock.unlock();
				//parent->lock.unlock();

				return;
			}

			// Atomic PlaceHolder
			Node *succ = maxSnapNode;
			Node *succParent = succ->getParent();

			// printf("%d: 6\n", id);
			bool lockedSuccParent = false;

			if (succParent != rightChild) {

				//succParent->lock.lock();
				// Atomic PlaceHolder
				if (maxSnapNode->getParent() != succParent || maxSnapNode->mark) {
					//succParent->lock.unlock();
					//if (lockedPred) minSnapNode->lock.unlock();
					//rightChild->lock.unlock();
					//leftChild->lock.unlock();
					//curr->lock.unlock();
					//parent->lock.unlock();
					continue;
				}

				lockedSuccParent = true;
			}

			// printf("%d: 7\n", id);
			//succ->lock.lock();
			// Atomic PlaceHolder
			if (maxSnapNode->leftSnap != curr || maxSnapNode->mark) {
				//succ->lock.unlock();
				//if (lockedSuccParent) succParent->lock.unlock();
				//if (lockedPred) minSnapNode->lock.unlock();
				//rightChild->lock.unlock();
				//leftChild->lock.unlock();
				//curr->lock.unlock();
				//parent->lock.unlock();
				continue;
			}

			// std::cout<<id<<": succParent :"<<succParent->getData()<<"\n";
			// printf("%d: 8\n", id);
			bool lockedSuccRightSnap = false;
			bool lockedSuccRightChild = false;

			// Atomic PlaceHolder
			Node *succRightChild = succ->getRight();
			Node *succRightSnapshot = succ->rightSnap;

			// Atomic PlaceHolder
			if (succRightChild != nullptr) {

				// printf("%d: 9\n", id);
				//succRightChild->lock.lock();
				lockedSuccRightChild = true;

				// printf("%d: 10\n", id);
				succRightSnapshot = succ->rightSnap;
				if (succRightSnapshot != succRightChild) {
					//succRightSnapshot->lock.lock();
					lockedSuccRightSnap = true;
				}

				// printf("%d: 11\n", id);
				if (succRightSnapshot->leftSnap != succ || succRightSnapshot->mark) {

					// Unlock all
					//if (lockedSuccRightSnap) succRightSnapshot->lock.unlock();
					//if (lockedSuccRightChild) succRightChild->lock.unlock();
					//succ->lock.unlock();
					//if (lockedSuccParent) succParent->lock.unlock();
					//if (lockedPred) minSnapNode->lock.unlock();
					//rightChild->lock.unlock();
					//leftChild->lock.unlock();
					//curr->lock.unlock();
					//parent->lock.unlock();
					continue;
				}
			}
			// printf("%d: 12\n", id);
			// Mark the node
			// Atomic PlaceHolder
			curr->mark = true;

			succ->setRight(rightChild);
			rightChild->setParent(succ);

			succ->setLeft(leftChild);
			leftChild->setParent(succ);

			succ->setParent(parent);

			// Atomic PlaceHolder
			if (parentIsLarger) {
				parent->setLeft(succ);
			}
			else {
				parent->setRight(succ);
			}
			// Atomic PlaceHolder
			succParent->setLeft(succRightChild);
			// Atomic PlaceHolder
			if (succRightChild != nullptr)
				succRightChild->setParent(succParent);

			// Atomic PlaceHolder
			// Update Snaps
			succ->rightSnap = succRightSnapshot;
			succRightSnapshot->leftSnap = succ;
			// Atomic PlaceHolder
			succ->leftSnap = minSnapNode;
			minSnapNode->rightSnap = succ;

			// Unlock All
			//if (lockedSuccRightSnap) succRightSnapshot->lock.unlock();
			//if (lockedSuccRightChild) succRightChild->lock.unlock();
			//succ->lock.unlock();
			//if (lockedSuccParent) succParent->lock.unlock();
			//if (lockedPred) minSnapNode->lock.unlock();
			//rightChild->lock.unlock();
			//leftChild->lock.unlock();
			//curr->lock.unlock();
			//parent->lock.unlock();

			return;
		}
	}
}

bool BinarySearchTree::contains(int const &data) {

	bool restart = false;
	while (true) {
		Node *curr;
		__transaction_atomic{
			curr = root;
		}
		int field = nextField(curr, data);
		Node *next;
		// traverse
		__transaction_atomic{
			next = curr->get(field);
		}
			while (true) {
				__transaction_atomic{
					if (next == nullptr) {
						break;
					}
				}
				curr = next;
				field = nextField(curr, data);

				// We have found node
				if (field == HERE) {

					__transaction_atomic{
						// If marked then break from first while loop and restart
						if (curr->mark) {
							restart = true;
							break;
						}
					}
						// Only executed if curr is not marked
					return curr;
				}
				__transaction_atomic{
					next = curr->get(field);
				}
			}

		if (restart == true) {
			restart = true;
			continue;
		}

		// grab snapshot
		// check if restart is needed
		bool goLeft = (data < curr->getData() ? true : false);
		Node* snapShot;
		//Purposely separated from the normal implementation for better performance
		__transaction_atomic{
			if (curr->mark) {
				continue;
			}
		snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
		}

			if ((goLeft && (data < snapShot->getData())) ||
				(!goLeft && (data > snapShot->getData()))) {
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
    std::cout<<curr->getData()<<": ("<<curr->leftSnap->getData()<<",";
    std::cout<<curr->rightSnap->getData()<<")\n";
    if (curr->getLeft()!=nullptr) q.push(curr->getLeft());
    if (curr->getRight()!=nullptr) q.push(curr->getRight());
  }
}

void doInserts(BinarySearchTree bst, int numberOfItems, int insertArray[]){
	//int numberOfTestCases = 100;
	//range = numberOfTestCases * (index + 1);
	for (int i = 0; i < numberOfItems; ++i) {
		bst.insert(insertArray[i]);
		
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

bool check(BinarySearchTree bst) {
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

void routine_1(BinarySearchTree &bst, int id, int n_threads,
	std::vector<int> keys) {

	int count = 0;
	for (int i = 1 * id; i<keys.size(); i += n_threads) {
		// printf("Thread %d: insert #%d\n", id, count);
		bst.insert(keys.at(i));
		// count++;
	}
	// std::cout<<"Check "<<check(bst)<<"!"<<std::endl;
	// bst.remove(-10, id);
	int increment = n_threads;
	for (int i = 1 * id; i<keys.size() / 2; i++) {
		// std::cout<<i<<std::endl;
		bst.remove(keys.at(i), id);
		// printf("Thread %d: remove #%d\n", id, count);
		// count++;
	}
}

int main(int argc, char **argv) {

	std::cout << "here" << std::endl;
	if (argc != 2) {
		std::cout << "Please enter correct number of arguments!" << std::endl;
		return -1;
	}

	int n_threads = std::atoi(argv[1]);
	std::thread threads[n_threads];

	std::vector<int> keys = init_list_ints(200000);
	BinarySearchTree bst = BinarySearchTree();
	for (int i = 0; i<n_threads; i++) {
		threads[i] = std::thread(routine_1, std::ref(bst), i, n_threads, std::ref(keys));
	}
	for (int i = 0; i<n_threads; i++) {
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