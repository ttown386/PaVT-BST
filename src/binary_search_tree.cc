
#include <limits>
#include <iostream>
#include <algorithm>
#include <thread>

#include <PaVT/binary_search_tree.h>

namespace pavt {

thread_local pavt::LockManager* BinarySearchTree::lock_manager = new pavt::LockManager();

base::BinaryTree::Node* BinarySearchTree::getRoot() {
  return root;
}

BinarySearchTree::Node* BinarySearchTree::getMinSentinel() {
  return minSentinel;
}

BinarySearchTree::Node* BinarySearchTree::getMaxSentinel() {
  return maxSentinel;
}

int BinarySearchTree::nextField(BinaryTree::Node *node, int const &key) {

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
BinarySearchTree::Node *BinarySearchTree::traverse(Node *node, int const &key) {
  bool restart = false;
  while (true) {

    Node *curr = node;
    int field = nextField(curr, key);
    Node *next = (Node*) curr->get(field);
    while (next != nullptr) {
      curr = next;

      field = nextField(curr, key);
      // We have found node
      if (field == HERE) {
        lock(curr);
        // If marked then break from first while loop and restart
        if (curr->mark) {
          unlock();
          restart = true;
          break;
        }

        // Only executed if curr is not marked
        return curr;
      }
      next = (Node*)curr->get(field); 
    }

    if (restart == true) {
      restart = false;
      continue;
    }
    lock(curr);
    // grab snapshot
    // check if restart is needed
    bool goLeft = (key < curr->getKey() ? true : false);
    Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
    if (curr->mark || 
          (goLeft && (key <= snapShot->getKey())) ||
      (!goLeft && (key >= snapShot->getKey()))) {
      unlock();
      continue;
    }

    return curr;
  }
}

/**
 * PaVTBST::contains Returns true if tree contains node and false otherwise
 * @param  key key to search for
 * @return      A boolean value
 */
bool BinarySearchTree::contains(int const &key) {
  bool restart = false;
  while (true) {

    Node *curr = (Node*)root;
    int field = nextField(curr, key);

    // traverse
    Node *next = (Node*)curr->get(field);
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
      next = (Node*)curr->get(field); 
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
      continue;
    }

    return false;
  }
}

void BinarySearchTree::lock(Node* node) {
  BinarySearchTree::lock_manager->Lock(node);
}

bool BinarySearchTree::tryLock(Node* node) {
  return BinarySearchTree::lock_manager->TryLock(node);
}

void BinarySearchTree::unlock() {
  BinarySearchTree::lock_manager->Unlock();
}

void BinarySearchTree::unlockAll() {
  BinarySearchTree::lock_manager->UnlockAll();
}

BinarySearchTree::Node* BinarySearchTree::insert(Node* node) {

  // Continue to attempt insertion
  while (true) {
    // traverse and lock node
    Node *curr = traverse((Node*)root, node->getKey());
  
    // We have a duplicate
    if (curr->getKey()== node->getKey()) {
      unlockAll();
      return nullptr;
    }
    
    // No longer a leaf node
    if (
        (node->getKey() > curr->getKey() && curr->right!=nullptr) ||
        (node->getKey() < curr->getKey() && curr->left!=nullptr)) {
      unlockAll();
      continue;
    }
    
    // Insert node and update parent
    node->parent = (curr);

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

      curr->left = (node);

    // Otherwise set right pointer and update snaps
    } else {
      node->leftSnap = curr;
      node->rightSnap = snapshot;

      snapshot->leftSnap = node;
      curr->rightSnap = node;

      curr->right = (node);
    }

    // Unlock
    unlockAll();

    return curr;
  }
}

std::pair<BinarySearchTree::Node*, BinarySearchTree::Node*>*
BinarySearchTree::remove(Node* node, const int& key) {

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
    Node *parent = (Node*)curr->parent;
    if (!tryLock(parent)) {
      unlockAll();
      continue;
    }

    // Some other thread has gone and changed things around
    // Got to check if we already got removed otherwise unlock restart
    if (parent != curr->parent) {
      unlockAll();
      if (curr->mark) {
        return new std::pair<Node*, Node*>(nullptr, nullptr);
      }
      continue;
    }

    Node *leftChild = (Node*)curr->left;
    Node *rightChild = (Node*)curr->right;
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
        parent->left = (nullptr);
        parent->leftSnap = minSnapNode;
        minSnapNode->rightSnap = parent;
      } else {
        parent->right = (nullptr);
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
      if (parent->left==curr) {
        parent->left = (currChild);
      } else {
        parent->right = (currChild);
      }

      currChild->parent = (parent);

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
      if (rightChild->left == nullptr) {

        curr->mark = true;

        // Updated pointers
        rightChild->left = (leftChild);
        leftChild->parent = (rightChild);
        rightChild->parent = (parent);

        if (parent->left==curr) {
          parent->left = (rightChild);
        } else {
          parent->right = (rightChild);
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
        Node *succParent = (Node*)succ->parent;

        // Successor's parent is no the right child
        if (succParent!=rightChild) {
          lock(succParent);

          if (maxSnapNode->parent != succParent || maxSnapNode->mark) {
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
        Node *succRightChild = (Node*)succ->right;
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

        succ->right = (rightChild);
        rightChild->parent = (succ);

        succ->left = (leftChild);
        leftChild->parent = (succ);

        succ->parent = (parent);

        
        if (parentIsLarger) {
          parent->left = (succ);
        } else {
          parent->right = (succ);
        }

        succParent->left = (succRightChild);

        if (succRightChild!=nullptr)
          succRightChild->parent = (succParent);
        
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
} // namespace pavt