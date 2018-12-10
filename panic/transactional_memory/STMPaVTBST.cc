//
// Created by ttown on 9/30/2018.
//

#include "STMPaVTBST.h"
#include <cmath>
#include <limits>
#include <algorithm>

const int MAXBF = 1;
const int MINBF = -1;
const int iMin = std::numeric_limits<int>::min();
const int iMax = std::numeric_limits<int>::max();
const int LEFT = 0;
const int RIGHT = 1;
const int HERE = 2;

STMPaVTBST::STMPaVTBST(bool isAvl) {
  // AVL condition
  this->isAvl = isAvl;

  // Bounds
  minSentinel = new STMNode(iMin);
  maxSentinel = new STMNode(iMax);

  // tree structure
  maxSentinel->setParent(minSentinel);
  minSentinel->setRight(maxSentinel);

  // Snapshots
  maxSentinel->leftSnap = minSentinel;
  minSentinel->rightSnap = maxSentinel;

  this->root = maxSentinel;
}

STMPaVTBST::~STMPaVTBST() {
  delete minSentinel;
}

STMNode *STMPaVTBST::getMinSentinel() {
  return minSentinel;
}

STMNode *STMPaVTBST::getRoot() {
  return root;
}

STMNode *STMPaVTBST::getMaxSentinel() {
  return maxSentinel;
}

int STMPaVTBST::nextField(STMNode *node, int const &data) {

  // c1(node, data) = L
  if (data<node->getKey()) return LEFT;

  // c2(node, data) = R
  if (data>node->getKey()) return RIGHT;

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
STMNode *STMPaVTBST::traverse(STMNode *node, int const &data) {
  bool restart = false;
  while (true) {

    STMNode *curr = root;
    int field = nextField(curr, data);

    // traverse
    STMNode *next = curr->get(field);
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
void STMPaVTBST::insert(int const &data) {

  // Otherwise we traverse
  while (true) {
    STMNode *curr = traverse(root, data);
    __transaction_atomic{

      // check if the node
      if (curr->getKey() == data && !curr->mark) {
        return;
      }

      // grab snapshot
      // check if restart is needed
      bool goLeft = (data < curr->getKey() ? true : false);
      STMNode *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
      if (curr->mark ||
        (goLeft && (data <= snapShot->getKey())) ||
        (!goLeft && (data >= snapShot->getKey()))) {
        continue;
      }

      // No longer a leaf node
      if (
        (data > curr->getKey() && curr->getRight() != nullptr) ||
        (data < curr->getKey() && curr->getLeft() != nullptr)) {
        continue;
      }

      STMNode *newNode = new STMNode(data);
      newNode->setParent(curr);

      // Copy snaps from parent
      bool parentIsLarger = data < curr->getKey();
      STMNode *snapshot = (parentIsLarger ? curr->leftSnap : curr->rightSnap);

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
void STMPaVTBST::remove(int const &data) {

  STMNode *maxSnapNode;
  STMNode *minSnapNode;

  STMNode *toBalance1 = nullptr;
  STMNode *toBalance2 = nullptr;

  while (true) {
    
    STMNode *curr = traverse(root, data);

    // Apply atomic transaction
    __transaction_atomic{

      bool goLeft = (data < curr->getKey() ? true : false);
      STMNode *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
      if (curr->mark ||
        (goLeft && (data <= snapShot->getKey())) ||
        (!goLeft && (data >= snapShot->getKey()))) {
        continue;
      }

      // Lock all nodes
      STMNode *parent = curr->getParent();

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
      if (curr->getKey() != data) {
        return;
      }

      
      STMNode *leftChild = curr->getLeft();
      STMNode *rightChild = curr->getRight();
      bool parentIsLarger = (parent->getKey() > data ? true : false);

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
        STMNode *currChild = (hasRightChild) ? rightChild : leftChild;

        //Another possible section
        minSnapNode = curr->leftSnap;
        maxSnapNode = curr->rightSnap;

        STMNode *snapshot = (hasRightChild) ? maxSnapNode : minSnapNode;

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

        /* STMNode with where the right child's left node is null */
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
          STMNode *succ = maxSnapNode;
          STMNode *succParent = succ->getParent();

          if (succParent != rightChild) {
            if (maxSnapNode->getParent() != succParent || maxSnapNode->mark) {
              continue;
            }
          }

          if (maxSnapNode->leftSnap != curr || maxSnapNode->mark) {
            continue;
          }

          STMNode *succRightChild = succ->getRight();
          STMNode *succRightSnapshot = succ->rightSnap;

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
bool STMPaVTBST::contains(int const &data) {
  bool restart = false;
  while (true) {

    STMNode *curr = root;
    int field = nextField(curr, data);

    // traverse
    STMNode *next = curr->get(field);
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
    bool goLeft = (data < curr->getKey() ? true : false);
    __transaction_atomic{
      STMNode *snapShot = (goLeft? curr->leftSnap : curr->rightSnap);
      if (curr->mark || (goLeft && (data < snapShot->getKey()))||
          (!goLeft &&(data > snapShot->getKey()))) {
        continue;
      }
    }
    return false;
  }
}

// Rotates node to the left. Child becomes nodes parent.
void STMPaVTBST::rotateLeft(STMNode *child, STMNode *node, STMNode *parent) {

  // Grab the nodes right child
  STMNode *newRoot = child;

  // Give node the left child of the rotated node since the
  // key is greater than node
  STMNode *temp = newRoot->getLeft();
  node->setRight(temp);

  // The node's right child (temp) now moves up to take the place of
  // node
  newRoot->setLeft(node);

  // Update parents
  if(temp!=nullptr) temp->setParent(node);

  STMNode *rootParent = parent;

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
void STMPaVTBST::rotateRight(STMNode *child, STMNode *node, STMNode *parent) {

  // Grab the nodes left child
  STMNode* newRoot = child;

  // Give node the left child of newRoot since the key
  // is less than node
  STMNode *temp = newRoot->getRight();
  node->setLeft(temp);

  // The new Root moves up to take the place of node
  // Now set newNodes right pointer to node
  newRoot->setRight(node);

  // Update parents
  if(temp!=nullptr) temp->setParent(node);

  STMNode *rootParent = parent;
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
int STMPaVTBST::height(STMNode *node) {
  return (node == nullptr) ? -1 : node->getHeight();
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 */
void STMPaVTBST::rebalance(STMNode *node) {

  if (node==root) {
    return;
  }

  STMNode *parent = node->getParent();
  STMNode *curr = node;
  STMNode *child;
  STMNode *grandChild;

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

      STMNode *left = curr->getLeft();
      STMNode *right= curr->getRight();

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

        STMNode *childLeft = child->getLeft();
        STMNode *childRight = child->getRight();

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

        STMNode *childLeft = child->getLeft();
        STMNode *childRight = child->getRight();

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
