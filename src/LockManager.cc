#include "PaVT/lock_manager.h"

namespace pavt {

LockManager::LockManager() {}

LockManager::~LockManager() {}

void LockManager::lock(base::Node* node) {
  node->lock.lock();
  lock_stack.push(node);
}

base::Node* LockManager::unlock() {
  base::Node* top_node = lock_stack.top();
  top_node->lock.unlock();
  lock_stack.pop();
}

void LockManager::unlockAll() {
  while (!lock_stack.empty()) {
    this->unlock();
  }
}
}

