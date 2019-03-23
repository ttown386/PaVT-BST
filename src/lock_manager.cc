#include "PaVT/lock_manager.h"

namespace pavt {

LockManager::LockManager() {}

LockManager::~LockManager() {}

void LockManager::lock(base::Node* node) {
  node->lock.lock();
  lock_stack.push(node);
}

bool LockManager::tryLock(base::Node* node) {
  bool node_is_locked = node->lock.try_lock();
  if (node_is_locked) {
    lock_stack.push(node);
  }
  return node_is_locked;
}

base::Node* LockManager::unlock() {
  base::Node* top_node = lock_stack.top();
  top_node->lock.unlock();
  lock_stack.pop();
  return top_node;
}

void LockManager::unlockAll() {
  while (!lock_stack.empty()) {
    this->unlock();
  }
}
}

