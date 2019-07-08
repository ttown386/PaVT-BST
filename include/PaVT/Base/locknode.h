#ifndef TTREE_INCLUDE_PAVT_BASE_LOCKNODE_H_
#define TTREE_INCLUDE_PAVT_BASE_LOCKNODE_H_

#include <mutex>
namespace pavt {
namespace base {

struct LockNode {
  std::mutex lock;
  bool mark;
};
} // namespace base
} // namespace pavt
#endif // TTREE_INCLUDE_PAVT_BASE_LOCKNODE_H_