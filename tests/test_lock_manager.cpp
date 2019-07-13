#include <vector>
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ttree/base/locknode.h>
#include <ttree/pavt/bst.h>
#include <ttree/pavt/lock_manager.h>

namespace {

class TestLockManager : public testing::Test {
 public:
  pavt::LockManager lock_manager;
  virtual void SetUp() {
   lock_manager = pavt::LockManager();
  }

  virtual void TearDown() {
    lock_manager.UnlockAll();
  }
};

TEST_F(TestLockManager, LockingNode) {
  auto node1 = new pavt::base::LockNode();
  auto node2 = node1;

  ASSERT_TRUE(node2->lock.try_lock());
  node2->lock.unlock();

  lock_manager.Lock(node1);
  ASSERT_FALSE(node2->lock.try_lock());
  lock_manager.Unlock();

  ASSERT_TRUE(node2->lock.try_lock());
  node2->lock.unlock();

  delete node1;
}

TEST_F(TestLockManager, UnlockAll) {
  std::vector< pavt::base::LockNode* > test_nodes;
  std::fill(test_nodes.begin(), test_nodes.end(), new pavt::base::LockNode());

  for (auto node = test_nodes.begin(); node != test_nodes.end(); node++) {
    lock_manager.Lock(*node);
  }

  pavt::base::LockNode* node;
  for (std::size_t i = 0; i < test_nodes.size(); i++) {
    node = test_nodes[i];
    ASSERT_FALSE(node->lock.try_lock());
  }


  lock_manager.UnlockAll();
  for (std::size_t i = 0; i < test_nodes.size(); i++) {
    node = test_nodes[i];
    ASSERT_TRUE(node->lock.try_lock());
    node->lock.unlock();
    delete node;
  }
}
} // namespace
