//
// Created by ttown on 10/6/2018.
//
#include <vector>
#include <algorithm>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "UtilitiesBST.h"
#include "PaVT/PaVTBST.h"

namespace {

class TestPaVTBST : public testing::Test {
 protected:
  const int max_val = std::numeric_limits<int>::max();
  PaVTBST *balanced_bst;
  std::vector<int> balanced_preorder; 
  std::vector<int> balanced_inorder;

 public:
  void SetUp() {
    balanced_preorder = {max_val, 40, 20, 10, 30, 60, 50, 70};
    balanced_inorder = {10, 20, 30, 40, 50, 60, 70, max_val};
    balanced_bst = init_balanced_bst();
  }

  void TearDown() {
    delete balanced_bst;
  }

  void insert_vector_into_tree(PaVTBST* tree, 
                               const std::vector<int>& node_list) {
    for (std::size_t i=0; i < node_list.size(); i++) {
      tree->insert(node_list[i]);
    }
  }

 private:
  PaVTBST* init_balanced_bst() {
    PaVTBST* bst = new PaVTBST();
    for (std::size_t i = 0; i < balanced_preorder.size(); i++) {
      bst->insert(balanced_preorder[i]);
    }
    return bst;
  }
};


TEST_F(TestPaVTBST, Insert) {
  PaVTBST* bst = new PaVTBST();
  std::vector<int> inorder_traversal = {-5, 6, 7, 10, max_val};
  std::vector<int> preorder_traversal = {max_val, 7, -5, 6, 10}; 
  for (std::size_t i = 0; i < preorder_traversal.size(); i++) {
    bst->insert(preorder_traversal[i]);
  }
  std::vector<int> inorder_traversal_v = inOrderTraversal(*bst);
  std::vector<int> preorder_traversal_v = preOrderTraversal(*bst);
  delete bst;
  EXPECT_THAT(inorder_traversal, testing::ContainerEq(inorder_traversal_v));
  EXPECT_THAT(preorder_traversal, testing::ContainerEq(preorder_traversal_v));
}


TEST_F(TestPaVTBST, InsertAlreadyExists) {
  std::vector<int> first_preorder = inOrderTraversal(*balanced_bst);
  insert_vector_into_tree(balanced_bst, balanced_preorder);
  std::vector<int> second_preorder = inOrderTraversal(*balanced_bst);
  EXPECT_THAT(first_preorder, testing::ContainerEq(second_preorder));
}


TEST_F(TestPaVTBST, InsertDoesntAddSentinelNodes) {
  ;
}


TEST_F(TestPaVTBST, Contains) {
  int val_1 = 70;
  int val_2 = 200;
  bool found_1 = balanced_bst->contains(val_1);
  bool found_2 = balanced_bst->contains(val_2);
  ASSERT_TRUE(found_1);
  ASSERT_TRUE(!found_2);
}


TEST_F(TestPaVTBST, ContainsDoesntFindSentielNodes) {
  ;
}


TEST_F(TestPaVTBST, DeleteLeaf) {
  int node_val = 70;
  balanced_bst->remove(node_val);
  balanced_preorder.pop_back();
  auto position = std::find(balanced_inorder.begin(), 
                            balanced_inorder.end(),
                            node_val);
  balanced_inorder.erase(position);
  std::vector<int> preorder = preOrderTraversal(*balanced_bst);
  std::vector<int> inorder = inOrderTraversal(*balanced_bst);
  EXPECT_THAT(balanced_preorder, testing::ContainerEq(preorder));
  EXPECT_THAT(balanced_inorder, testing::ContainerEq(inorder));
}


TEST_F(TestPaVTBST, DeleteNodeWithLeftChild) {
  int insert_val = 45;
  int remove_val = 50;
  balanced_bst->insert(insert_val);
  balanced_bst->remove(remove_val);
  std::vector<int> preorder = preOrderTraversal(*balanced_bst);
  std::vector<int> inorder = inOrderTraversal(*balanced_bst);
  auto index_pre = find(balanced_preorder.begin(), balanced_preorder.end(), remove_val);
  auto index_in = find(balanced_inorder.begin(), balanced_inorder.end(), remove_val);
  *index_pre = insert_val;
  *index_in = insert_val;
  EXPECT_THAT(balanced_preorder, testing::ContainerEq(preorder));
  EXPECT_THAT(balanced_inorder, testing::ContainerEq(inorder));
}


TEST_F(TestPaVTBST, DeleteNodeWithRightChild) {
  int new_val = 80;
  int old_val = 70;
  std::vector<int> new_preorder {max_val, 40, 20, 10, 30, 60, 50, 80};
  std::vector<int> new_inorder {10, 20, 30, 40, 50, 60, 80, max_val};
  balanced_bst->insert(new_val);
  balanced_bst->remove(old_val);
  std::vector<int> preorder = preOrderTraversal(*balanced_bst);
  std::vector<int> inorder = inOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
}


TEST_F(TestPaVTBST, DeleteNodeWithTwoLeafs) {
  int old_val = 60;
  std::vector<int> new_preorder {max_val, 40, 20, 10, 30, 70, 50};
  std::vector<int> new_inorder {10, 20, 30, 40, 50, 70, max_val};
  balanced_bst->remove(old_val);
  std::vector<int> preorder = preOrderTraversal(*balanced_bst);
  std::vector<int> inorder = inOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));;
}


TEST_F(TestPaVTBST, DeleteNodeRightChildLeftSubtreeContainsSuccessorLeaf) {
  int new_val = 45;
  int old_val = 40;
  std::vector<int> new_preorder {max_val, 45, 20, 10, 30, 60, 50, 70};
  std::vector<int> new_inorder {10, 20, 30, 45, 50, 60, 70, max_val};
  balanced_bst->insert(new_val);
  balanced_bst->remove(old_val);
  std::vector<int> preorder = preOrderTraversal(*balanced_bst);
  std::vector<int> inorder = inOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
}


TEST_F(TestPaVTBST, DeleteNodeRightChildIsLeafSuccessorsParent) {
  int new_val = 65;
  int old_val = 60;
  std::vector<int> new_preorder {max_val, 40, 20, 10, 30, 65, 50, 70};
  std::vector<int> new_inorder {10, 20, 30, 40, 50, 65, 70, max_val};
  balanced_bst->insert(new_val);
  balanced_bst->remove(old_val);
  std::vector<int> preorder = preOrderTraversal(*balanced_bst);
  std::vector<int> inorder = inOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
}


TEST_F(TestPaVTBST, DeleteNodeSucessorHasRightChild) {
  int new_val_1 = 45;
  int new_val_2 = 47;
  int old_val = 40;
  std::vector<int> new_preorder {max_val, 45, 20, 10, 30, 60, 50, 47, 70};
  std::vector<int> new_inorder {10, 20, 30, 45, 47, 50, 60, 70, max_val};
  balanced_bst->insert(new_val_1);
  balanced_bst->insert(new_val_2);
  balanced_bst->remove(old_val);
  std::vector<int> preorder = preOrderTraversal(*balanced_bst);
  std::vector<int> inorder = inOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));

}


TEST_F(TestPaVTBST, NoneDeleted) {
  int old_val = -100;
  balanced_bst->remove(old_val);
  std::vector<int> preorder = preOrderTraversal(*balanced_bst);
  std::vector<int> inorder = inOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(balanced_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(balanced_inorder));
}


TEST_F(TestPaVTBST, DeleteDoesntRemoveSentielNodes) {
  ;
}
}