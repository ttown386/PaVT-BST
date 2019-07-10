//
// Created by ttown on 10/6/2018.
//
#include <vector>
#include <algorithm>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <PaVT/sequential/bst.h>

using namespace pavt;
namespace {

class TestSeqBST : public testing::Test {
 protected:
  const int max_val = std::numeric_limits<int>::max();
  seq::BST *balanced_bst;
  std::vector<int> balanced_preorder; 
  std::vector<int> balanced_inorder;

  void SetUp() {
    balanced_preorder = {max_val, 40, 20, 10, 30, 60, 50, 70};
    balanced_inorder = {10, 20, 30, 40, 50, 60, 70, max_val};
    balanced_bst = init_balanced_bst();
  } 

  void TearDown() {
    delete balanced_bst;
  }

  void insert_vector_into_tree(seq::BST* tree, 
                               const std::vector<int>& node_list) {
    for (std::size_t i=0; i < node_list.size(); i++) {
      tree->Insert(node_list[i]);
    }
  }

 private:
  seq::BST* init_balanced_bst() {
    seq::BST* bst = new seq::BST();
    for (std::size_t i = 0; i < balanced_preorder.size(); i++) {
      bst->Insert(balanced_preorder[i]);
    }
    return bst;
  }
  void print_traversal(std::vector<int>& trav) {
    for (auto i : trav) {
      std::cout << i << " ";
    }
    std::cout << std::endl;
  }
};


TEST_F(TestSeqBST, Insert) {
  seq::BST* bst = new seq::BST();
  std::vector<int> inorder_traversal = {-5, 6, 7, 10, max_val};
  std::vector<int> preorder_traversal = {max_val, 7, -5, 6, 10}; 
  for (std::size_t i = 0; i < preorder_traversal.size(); i++) {
    bst->Insert(preorder_traversal[i]);
  }
  std::vector<int> inorder_traversal_v = InOrderTraversal(*bst);
  std::vector<int> preorder_traversal_v = PreOrderTraversal(*bst);
  delete bst;
  EXPECT_THAT(inorder_traversal, testing::ContainerEq(inorder_traversal_v));
  EXPECT_THAT(preorder_traversal, testing::ContainerEq(preorder_traversal_v));
}


TEST_F(TestSeqBST, InsertAlreadyExists) {
  std::vector<int> first_preorder = InOrderTraversal(*balanced_bst);
  insert_vector_into_tree(balanced_bst, balanced_preorder);
  std::vector<int> second_preorder = InOrderTraversal(*balanced_bst);
  EXPECT_THAT(first_preorder, testing::ContainerEq(second_preorder));
}


TEST_F(TestSeqBST, Contains) {
  int val_1 = 70;
  int val_2 = 200;
  bool found_1 = balanced_bst->Contains(val_1);
  bool found_2 = balanced_bst->Contains(val_2);
  ASSERT_TRUE(found_1);
  ASSERT_TRUE(!found_2);
}


TEST_F(TestSeqBST, DeleteLeaf) {
  int node_val = 70;
  balanced_bst->Remove(node_val);
  balanced_preorder.pop_back();
  auto position = std::find(balanced_inorder.begin(), 
                            balanced_inorder.end(),
                            node_val);
  balanced_inorder.erase(position);
  std::vector<int> preorder = PreOrderTraversal(*balanced_bst);
  std::vector<int> inorder = InOrderTraversal(*balanced_bst);
  EXPECT_THAT(balanced_preorder, testing::ContainerEq(preorder));
  EXPECT_THAT(balanced_inorder, testing::ContainerEq(inorder));
}


TEST_F(TestSeqBST, DeleteNodeWithLeftChild) {
  int Insert_val = 45;
  int Remove_val = 50;
  balanced_bst->Insert(Insert_val);
  balanced_bst->Remove(Remove_val);
  std::vector<int> preorder = PreOrderTraversal(*balanced_bst);
  std::vector<int> inorder = InOrderTraversal(*balanced_bst);
  auto index_pre = find(balanced_preorder.begin(), balanced_preorder.end(), Remove_val);
  auto index_in = find(balanced_inorder.begin(), balanced_inorder.end(), Remove_val);
  *index_pre = Insert_val;
  *index_in = Insert_val;
  EXPECT_THAT(balanced_preorder, testing::ContainerEq(preorder));
  EXPECT_THAT(balanced_inorder, testing::ContainerEq(inorder));
}


TEST_F(TestSeqBST, DeleteNodeWithRightChild) {
  int new_val = 80;
  int old_val = 70;
  std::vector<int> new_preorder {max_val, 40, 20, 10, 30, 60, 50, 80};
  std::vector<int> new_inorder {10, 20, 30, 40, 50, 60, 80, max_val};
  balanced_bst->Insert(new_val);
  balanced_bst->Remove(old_val);
  std::vector<int> preorder = PreOrderTraversal(*balanced_bst);
  std::vector<int> inorder = InOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
}


TEST_F(TestSeqBST, DeleteNodeWithTwoLeafs) {
  int old_val = 60;
  std::vector<int> new_preorder {max_val, 40, 20, 10, 30, 70, 50};
  std::vector<int> new_inorder {10, 20, 30, 40, 50, 70, max_val};
  balanced_bst->Remove(old_val);
  std::vector<int> preorder = PreOrderTraversal(*balanced_bst);
  std::vector<int> inorder = InOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));;
}


TEST_F(TestSeqBST, DeleteNodeRightChildLeftSubtreeContainsSuccessorLeaf) {
  int new_val = 45;
  int old_val = 40;
  std::vector<int> new_preorder {max_val, 45, 20, 10, 30, 60, 50, 70};
  std::vector<int> new_inorder {10, 20, 30, 45, 50, 60, 70, max_val};
  balanced_bst->Insert(new_val);
  balanced_bst->Remove(old_val);
  std::vector<int> preorder = PreOrderTraversal(*balanced_bst);
  std::vector<int> inorder = InOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
}


TEST_F(TestSeqBST, DeleteNodeRightChildIsLeafSuccessorsParent) {
  int new_val = 65;
  int old_val = 60;
  std::vector<int> new_preorder {max_val, 40, 20, 10, 30, 65, 50, 70};
  std::vector<int> new_inorder {10, 20, 30, 40, 50, 65, 70, max_val};
  balanced_bst->Insert(new_val);
  balanced_bst->Remove(old_val);
  std::vector<int> preorder = PreOrderTraversal(*balanced_bst);
  std::vector<int> inorder = InOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
}


TEST_F(TestSeqBST, DeleteNodeSucessorHasRightChild) {
  int new_val_1 = 45;
  int new_val_2 = 47;
  int old_val = 40;
  std::vector<int> new_preorder {max_val, 45, 20, 10, 30, 60, 50, 47, 70};
  std::vector<int> new_inorder {10, 20, 30, 45, 47, 50, 60, 70, max_val};
  balanced_bst->Insert(new_val_1);
  balanced_bst->Insert(new_val_2);
  balanced_bst->Remove(old_val);
  std::vector<int> preorder = PreOrderTraversal(*balanced_bst);
  std::vector<int> inorder = InOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));

}


TEST_F(TestSeqBST, NoneDeleted) {
  int old_val = -100;
  balanced_bst->Remove(old_val);
  std::vector<int> preorder = PreOrderTraversal(*balanced_bst);
  std::vector<int> inorder = InOrderTraversal(*balanced_bst);
  EXPECT_THAT(preorder, testing::ContainerEq(balanced_preorder));
  EXPECT_THAT(inorder, testing::ContainerEq(balanced_inorder));
}
}