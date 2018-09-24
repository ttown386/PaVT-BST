//
// Created by ttown on 9/23/2018.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>

#include "Node.h"

namespace {
class TestNode : public testing::Test {
 protected:
  Node * node;
 public:

   virtual void SetUp();
   virtual void TearDown();
};
}

void TestNode::SetUp() {
  node = new Node(5);
}

void TestNode::TearDown() {
  delete node;
}

TEST_F(TestNode, setData) {

  ASSERT_EQ(5, node->getData());
}

TEST_F(TestNode, inserting) {

  int val1 = 7;
  int val2 = 1;

  Node *test1 = new Node(val1);
  Node *test2 = new Node(val2);

  node->setLeft(test2);
  node->setRight(test1);

  ASSERT_EQ(val2, node->getLeft()->getData());
  ASSERT_EQ(val1, node->getRight()->getData());
}
