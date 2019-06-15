
#include <limits>
#include <iostream>
#include <algorithm>
#include <thread>

#include <PaVT/Base/binary_tree.h>

namespace pavt {
namespace base {

thread_local pavt::LockManager* BinaryTree::lock_manager = new pavt::LockManager();

pavt::Node *BinaryTree::getRoot() {
  return root;
}

pavt::Node *BinaryTree::getMinSentinel() {
  return minSentinel;
}

pavt::Node *BinaryTree::getMaxSentinel() {
  return maxSentinel;
}

int BinaryTree::nextField(pavt::Node *node, int const &key) {

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
pavt::Node *BinaryTree::traverse(pavt::Node *node, int const &key) {
  bool restart = false;
  while (true) {

    pavt::Node *curr = node;
    int field = nextField(curr, key);
    pavt::Node *next = curr->get(field);
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
      next = curr->get(field); 
    }

    if (restart == true) {
      restart = false;
      continue;
    }
    lock(curr);
    // grab snapshot
    // check if restart is needed
    bool goLeft = (key < curr->getKey() ? true : false);
    pavt::Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
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
bool BinaryTree::contains(int const &key) {
  bool restart = false;
  while (true) {

    pavt::Node *curr = root;
    int field = nextField(curr, key);

    // traverse
    pavt::Node *next = curr->get(field);
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
    pavt::Node *snapShot = (goLeft ? curr->leftSnap : curr->rightSnap);
    if (curr->mark || 
      (goLeft && (key <= snapShot->getKey())) ||
      (!goLeft && (key >= snapShot->getKey()))) {
      continue;
    }

    return false;
  }
}

void BinaryTree::lock(Node* node) {
  BinaryTree::lock_manager->lock(node);
}

bool BinaryTree::tryLock(Node* node) {
  return BinaryTree::lock_manager->tryLock(node);
}

void BinaryTree::unlock() {
  BinaryTree::lock_manager->unlock();
}

void BinaryTree::unlockAll() {
  BinaryTree::lock_manager->unlockAll();
}
} // namespace base
} // namespace pavt