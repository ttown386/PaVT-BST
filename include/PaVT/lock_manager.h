#ifndef PAVT_LOCK_MANAGER_H
#define PAVT_LOCK_MANAGER_H

#include <stack>

#include "PaVT/Base/node.h"
#include "PaVT/PaVTBST.h"

namespace pavt {

class LockManager {
 public:
  LockManager();
  ~LockManager();
  void lock(base::Node* node);
  base::Node* unlock();
  void unlockAll();
 protected:
  std::stack<base::Node* > lock_stack;
};
} // pavt
#endif // PAVT_LOCK_MANGER_H_