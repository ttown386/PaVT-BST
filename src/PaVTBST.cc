//
// Created by ttown on 9/30/2018.
//

#include <limits>
#include <iostream>
#include <algorithm>
#include <thread>

#include <PaVT/Base/node.h>
#include <PaVT/PaVTBST.h>

namespace pavt {
/**
 * PaVTBST::insert Insert new node into tree. If tree contains node
 * no node is inserted
 *
 * 
 * @param key key to be inserted into tree
 */
void PaVTBST::insert(const int& key) {
  Node* new_node = new Node(key);
  Node* return_node = insert(new_node);
  if (return_node == nullptr) {
    delete new_node;
  } else {
    if (isAvl) rebalance(return_node);
  }
}

Node* PaVTBST::insert(Node* node) {

  // Continue to attempt insertion
  while (true) {

    // traverse and lock node
    Node *curr = traverse(root, node->getKey());
  
    // We have a duplicate
    if (curr->getKey()== node->getKey()) {
      unlockAll();
      return nullptr;
    }
    
    // No longer a leaf node
    if (
        (node->getKey() > curr->getKey() && curr->getRight()!=nullptr) ||
        (node->getKey() < curr->getKey() && curr->getLeft()!=nullptr)) {
      unlockAll();
      continue;
    }
    
    // Insert node and update parent
    node->setParent(curr);

    // Copy snaps from parent
    bool parentIsLarger = node->getKey() < curr->getKey();
    Node *snapshot = (parentIsLarger ? curr->leftSnap.load() : curr->rightSnap.load());


    // If parent is larger, set left pointer to new node
    // and update snaps
    if (parentIsLarger) {
      node->leftSnap = snapshot;
      node->rightSnap = curr;

      snapshot->rightSnap = node;
      curr->leftSnap = node;

      curr->setLeft(node);

    // Otherwise set right pointer and update snaps
    } else {
      node->leftSnap = curr;
      node->rightSnap = snapshot;

      snapshot->leftSnap = node;
      curr->rightSnap = node;

      curr->setRight(node);
    }

    // Unlock
    unlockAll();

    // Perform AVL rotations if applicable
    if (isAvl) {
      rebalance(curr);
    }
    return curr;
  }
}

/**
 * PaVTBST::remove Removes node from tree. If node is not present then the
 * call returns. 
 * @param key The key to be removed from the tree
 */
void PaVTBST::remove(const int& key) {
  auto balance_nodes = remove(root, key);
  if (isAvl && balance_nodes->first != nullptr) {
    rebalance(balance_nodes->first);
    if (balance_nodes->second != nullptr) rebalance(balance_nodes->second);
  } 
  delete balance_nodes;
}

std::pair<Node*, Node*>* PaVTBST::remove(Node* node, const int& key) {

  Node *maxSnapNode;
  Node *minSnapNode;

  Node *toBalance1 = nullptr;
  Node *toBalance2 = nullptr;
  
  // Continually attempt removal until call is returned
  while (true) {

    // Grab node
    Node *curr = traverse(node, key);

    // Already checked snapshots so return if current
    // node is not one to be deleted
    if (curr->getKey()!= key) {
      unlockAll();
      return new std::pair<Node*, Node*>(nullptr, nullptr);
    }
    
    // Lock Parent
    Node *parent = curr->getParent();
    if (!tryLock(parent)) {
      unlockAll();
      continue;
    }

    // Some other thread has gone and changed things around
    // Got to check if we already got removed otherwise unlock restart
    if (parent != curr->getParent()) {
      unlockAll();
      if (curr->mark) {
        return new std::pair<Node*, Node*>(nullptr, nullptr);
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
      unlockAll();
      toBalance1 = parent;

    } else if (leftChild==nullptr || rightChild==nullptr) {
    // if (leftChild !=nullptr) lock(leftChild);
    // if (rightChild !=nullptr) lock(rightChild);

    /* A node with at most 1 child */

      bool hasRightChild = leftChild == nullptr;
      Node *currChild = (hasRightChild) ? rightChild : leftChild;
      lock(currChild);

      // Load snaps
      minSnapNode = curr->leftSnap.load();
      maxSnapNode = curr->rightSnap.load();

      Node *snapshot =  (hasRightChild) ? maxSnapNode : minSnapNode;
      
      
      // if the snapshot of curr is not its child then lock
      // that node as its path can be altered
      if (snapshot!=currChild) {
        lock(snapshot);
      }

      // if the snapshot has changed unlock all and restart
      if ((hasRightChild && snapshot->leftSnap.load()!=curr) || 
          (!hasRightChild && snapshot->rightSnap.load()!=curr) ||
          snapshot->mark) {
        unlockAll();
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

      unlockAll();
      toBalance1 = parent;
    } else {
      lock(leftChild);
      lock(rightChild);

      /* Hard Cases */
      minSnapNode = curr->leftSnap.load();
      maxSnapNode = curr->rightSnap.load();

      
      // Lock if leftSnap is not the leftChild
      if (minSnapNode != leftChild) {
        lock(minSnapNode);
      } 

      // Check if the LeftSnapshot's right snapshot is the node
      // to be removed
      if (minSnapNode->rightSnap!=curr || minSnapNode->mark) {
        unlockAll();
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
        unlockAll();
        toBalance1 = rightChild; 

      } else {
        /* Hardest Case */
        Node *succ = maxSnapNode;
        Node *succParent = succ->getParent();

        // Successor's parent is no the right child
        if (succParent!=rightChild) {
          lock(succParent);

          if (maxSnapNode->getParent() != succParent || maxSnapNode->mark) {
            unlockAll();
            continue;
          }
        }

        // Lock successor
        lock(succ);
        if (maxSnapNode->leftSnap.load()!=curr || maxSnapNode->mark) {
          unlockAll();
          continue;
        }

        // Lock Right Child if successor has a right child
        // and rightSnap if it is not the right child
        Node *succRightChild = succ->getRight();
        Node *succRightSnapshot = succ->rightSnap.load();

        if (succRightChild!=nullptr)  {
          lock(succRightChild);
          succRightSnapshot = succ->rightSnap.load();

          if (succRightSnapshot!=succRightChild) {
            lock(succRightSnapshot);
          }
          // Check if it's left snap is still the successor
          if (succRightSnapshot->leftSnap.load()!=succ||succRightSnapshot->mark) {
            unlockAll();
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
        unlockAll();
        toBalance1 = succ;
        toBalance2 = succParent;
      }
    }
  }
  return new std::pair<Node*, Node*>(toBalance1, toBalance2);
}

// Rotates node to the left. Child becomes nodes parent.
void PaVTBST::rotateLeft(Node *child, Node *node, Node *parent) {

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
void PaVTBST::rotateRight(Node *child, Node *node, Node *parent) {

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
int PaVTBST::height(Node *node) {
  return (node == nullptr) ? -1 : node->getHeight();
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 */
void PaVTBST::rebalance(Node *node) {

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
} // namespace pavt