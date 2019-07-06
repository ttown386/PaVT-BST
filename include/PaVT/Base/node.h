#ifndef PAVT_BASE_NODE_H_
#define PAVT_BASE_NODE_H_

#include <mutex>
namespace pavt {
namespace base {

struct Node {
  std::mutex lock;
  bool mark;
};
} // namespace base
} // namespace pavt
#endif // PAVT_BASE_NODE_H_