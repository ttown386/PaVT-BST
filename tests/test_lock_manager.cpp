#include <vector>
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "UtilitiesBST.h"
#include "PaVT/Base/node.h"
#include "PaVT/PaVTBST.h"
#include "PaVT/lock_manager.h"

namespace {

class TestLockManager : public testing::Test {
 public:
  pavt::LockManager lock_manager;
  virtual void SetUp() {
   lock_manager = pavt::LockManager();
  }

  virtual void TearDown() {
    lock_manager.unlockAll();
  }
};

TEST_F(TestLockManager, LockingNode) {
  pavt::base::Node* node1 = new pavt::base::Node();
  pavt::base::Node* node2 = node1;

  ASSERT_TRUE(node2->lock.try_lock());
  node2->lock.unlock();

  lock_manager.lock(node1);
  ASSERT_FALSE(node2->lock.try_lock());
  lock_manager.unlock();

  ASSERT_TRUE(node2->lock.try_lock());
  node2->lock.unlock();

  delete node1;
}

TEST_F(TestLockManager, UnlockAll) {
  std::vector<pavt::base::Node* > test_nodes;
  std::fill(test_nodes.begin(), test_nodes.end(), new pavt::base::Node());

  for (auto node = test_nodes.begin(); node != test_nodes.end(); node++) {
    lock_manager.lock(*node);
  }

  pavt::base::Node* node;
  for (std::size_t i = 0; i < test_nodes.size(); i++) {
    node = test_nodes[i];
    ASSERT_FALSE(node->lock.try_lock());
  }


  lock_manager.unlockAll();
  for (std::size_t i = 0; i < test_nodes.size(); i++) {
    node = test_nodes[i];
    ASSERT_TRUE(node->lock.try_lock());
    node->lock.unlock();
    delete node;
  }
}
} // namespace
