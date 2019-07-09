#include <PaVT/pavt/lock_manager.h>

namespace pavt {

LockManager::LockManager() {}

LockManager::~LockManager() {}

void LockManager::Lock(base::LockNode* node) {
  node->lock.lock();
  _lock_stack.push(node);
}

bool LockManager::TryLock(base::LockNode* node) {
  bool node_is_locked = node->lock.try_lock();
  if (node_is_locked) {
    _lock_stack.push(node);
  }
  return node_is_locked;
}

base::LockNode* LockManager::Unlock() {
  auto top_node = _lock_stack.top();
  top_node->lock.unlock();
  _lock_stack.pop();
  return top_node;
}

void LockManager::UnlockAll() {
  while (!_lock_stack.empty()) {
    this->Unlock();
  }
}
}

