//
// Created by ttown on 9/30/2018.
//

#include "SerialStmBst.h"
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

SerialStmBst::SerialStmBst(bool isAvl) {
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

SerialStmBst::~SerialStmBst() {
  delete root;
}

Node *SerialStmBst::getRoot() {
  return root;
}

int SerialStmBst::nextField(Node *node, int const &data) {

  // c1(node, data) = L
  if (data<node->getData()) return LEFT;

  // c2(node, data) = R
  if (data>node->getData()) return RIGHT;

  // c3(node, data) = null;
  return HERE;
}

Node *SerialStmBst::traverse(Node *node, int const &data) {

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

    bool goLeft = (data < curr->getData() ? true : false);
		Node* snapShot = (goLeft? curr->leftSnap : curr->rightSnap);
		
		if (curr->mark || (goLeft && (data < snapShot->getData())) ||
				(!goLeft &&(data > snapShot->getData()))) {
			continue;
		}
    return curr;
  }
}

void SerialStmBst::insert(int const &data) {

  // Otherwise we traverse
  while (true) {
    Node *curr;
    __transaction_atomic {
      curr = traverse(root, data);
	
			// We have a duplicate
			if (curr->getData()== data) {
				//curr->lock.unlock();
				return;
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
      if (isAvl) rebalance(curr);
    }
    // std::cout<<"Finished insert"<<std::endl;
    // std::cout<<"Finished rebalancing"<<std::endl;
    return;
  }
}

void SerialStmBst::remove(int const &data) {

  Node *maxSnapNode;
  Node *minSnapNode;

  Node *toBalance1 = nullptr;
  Node *toBalance2 = nullptr;

  while (true) {
    __transaction_atomic{
      Node *curr = traverse(root, data);

    // Apply atomic transaction
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

        toBalance1 = parent;

        /* Hard Cases */ 
      } else {
        
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
          // Atomic PlaceHolder
          succ->leftSnap = minSnapNode;
          minSnapNode->rightSnap = succ; 
          toBalance1 = succ;
          toBalance2 = succParent;
        }
      }
      if (isAvl) {
        rebalance(toBalance1);
        if (toBalance2!=nullptr) rebalance(toBalance2);
      }
    }
    // std::cout<<"Finished removing"<<std::endl;
    // std::cout<<"Finished rebalancing"<<std::endl;
    return;
  }
}




bool SerialStmBst::contains(int const &data) {
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

void SerialStmBst::rotateLeft(Node *child, Node *node, Node *parent) {

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

void SerialStmBst::rotateRight(Node *child, Node *node, Node *parent) {

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
int SerialStmBst::height(Node *node) {
  return (node == nullptr) ? -1 : node->getHeight();
}


void SerialStmBst::rebalance(Node *node) {

  if (node==root) {
    return;
  }
  Node *curr = node;
  Node *parent;
  while(curr!=root) {
    parent = curr->getParent();
    __transaction_atomic {
    
      if (curr->getParent()!=parent) {
        if (curr->mark) {
          return;
        }
        parent = curr->getParent();
        continue;
      }
      // std::cout<<"b\n";
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

      Node *child;
      // The curr's right subtree is too tall
      if (bf < MINBF) {
        child = right;

        Node *childLeft = child->getLeft();
        Node *childRight = child->getRight();

        int childLeftHeight = height(childLeft);
        int childRightHeight = height(childRight);

        int childBf = childLeftHeight - childRightHeight;

        Node *grandChild = childLeft;

        if (childBf > 0) {
          rotateRight(grandChild, child, curr);
          rotateLeft(grandChild, curr, parent);
          curr = grandChild;

        } else {
          rotateLeft(child, curr, parent);
          curr = child;
        }

        // The curr's left subtree is too tall
      } else if (bf > MAXBF) {
        child = left;

        Node *childLeft = child->getLeft();
        Node *childRight = child->getRight();

        int childLeftHeight = height(childLeft);
        int childRightHeight = height(childRight);

        int childBf = childLeftHeight - childRightHeight;

        Node *grandChild = childRight;

        if (childBf < 0) {
          rotateLeft(grandChild, child, curr);
          rotateRight(grandChild, curr, parent);
          curr = grandChild;
        } else {
          rotateRight(child, curr, parent);
          curr = child;
        }

      } else {
        // Traverse back up tree
        curr = parent;
        parent = curr->getParent();
      }
    }
  }
}

/**
 *
 * UTILITY FUNCTIONS
 * 
 */

void inOrderTraversal(SerialStmBst &bst) {

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
void preOrderTraversal(SerialStmBst &bst) {

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

void printSnaps(SerialStmBst bst) {
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

bool check(SerialStmBst bst) {
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

void routine_1(SerialStmBst &bst, int id, int n_threads,
  std::vector<int> keys) {

  int count = 0;
  // std::cout<<"before"<<std::endl;
  for (int i = 1 * id; i<keys.size(); i += n_threads) {
    // printf("Thread %d: insert #%d\n", id, count);
    bst.insert(keys.at(i));
    // std::cout<<"insert"<<std::endl;
    // count++;
  }
  std::cout<<"after"<<std::endl;
  int increment = n_threads;
  // std::cout<<"here"<<std::endl;
  for (int i = 1 * id; i<keys.size(); i++) {
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
  
  SerialStmBst bst = SerialStmBst(true);
  for (int j=0; j<1; j++) {
    for (int i = 0; i<n_threads; i++) {
      threads[i] = std::thread(routine_1, std::ref(bst), i, n_threads, std::ref(keys));
    }
    for (int i = 0; i<n_threads; i++) {
      threads[i].join();
    }
  }

  printf("Preorder :");
  preOrderTraversal(bst);
  printf("Inorder :");
  inOrderTraversal(bst);
  printf("Snaps: \n");
  printSnaps(bst);
  printf("\n");
  std::cout<<"check "<<check(bst)<<std::endl;
}