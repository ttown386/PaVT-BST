#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "UtilitiesBST.h"
#include "PaVT/PaVTBST.h"

namespace {

class TestPaVTAVL : public testing::Test {
 protected:
  const int max_val = std::numeric_limits<int>::max();
  PaVTBST* avl;
  PaVTBST* small_avl;
  const std::vector<int> preorder_balanced {max_val, 4, 2, 6, 1, 3, 5, 7};
  const std::vector<int> inorder_balanced {1, 2, 3, 4, 5, 6, 7, max_val};
  const std::vector<int> preorder_small {max_val, 4, 10};
  const std::vector<int> inorder_small {4, 10, max_val};

 public:
  void SetUp() {
    avl = new PaVTBST(true);
    small_avl = init_small_tree();
  }

  void TearDown() {
    delete avl;
  }
  
  void insert_vector_into_tree(PaVTBST* tree, 
                               const std::vector<int>& node_list) {
    for (int i=0; i < node_list.size(); i++) {
      tree->insert(node_list[i]);
    }
  }

 private:
  PaVTBST* init_balanced_avl() {
    PaVTBST* bst = new PaVTBST(true);
    insert_vector_into_tree(bst, preorder_balanced);
    return bst;
  }

  PaVTBST* init_small_tree() {
    PaVTBST* bst = new PaVTBST(true);
    insert_vector_into_tree(bst, preorder_small);
    return bst;
  }
};


TEST_F(TestPaVTAVL, InsertSingleRightRotation) {
  int new_val = 14;
  const std::vector<int> new_preorder = {max_val, 10, 4, new_val};
  const std::vector<int> new_inorder = {4, 10, new_val, max_val}; 
  insert_vector_into_tree(small_avl, new_preorder);
  std::vector<int> inorder = inOrderTraversal(*small_avl);
  std::vector<int> preorder = preOrderTraversal(*small_avl);
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
}


TEST_F(TestPaVTAVL, InsertSingleLeftRotation) {
  int new_val = 14;
  const std::vector<int> new_preorder = {max_val, 4, 2, 1, 3, 10};
  const std::vector<int> new_inorder = {1, 2, 3, 4, 10, max_val}; 
  insert_vector_into_tree(small_avl, new_preorder);
  std::vector<int> inorder = inOrderTraversal(*small_avl);
  std::vector<int> preorder = preOrderTraversal(*small_avl);
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
}


TEST_F(TestPaVTAVL, AVLInsertLeftRightRot) {

  std::vector<int> avl_data{2, 1, 10, 11, 7, 9};
  std::vector<int> avl_preorder{max_val, 7, 2, 1, 10, 9, 11,};
  std::vector<int> avl_inorder(avl_preorder.begin(), avl_preorder.end());
  std::sort(avl_inorder.begin(), avl_inorder.end());

  PaVTBST* avl_tree = new PaVTBST(true);
  for (int i=0; i<avl_data.size(); i++) {
    avl_tree->insert(avl_data[i]);
  }
  std::vector<int> inorder = inOrderTraversal(*avl_tree);
  std::vector<int> preorder = preOrderTraversal(*avl_tree);
  delete avl_tree;

  EXPECT_THAT(inorder, testing::ContainerEq(avl_inorder));
  EXPECT_THAT(preorder, testing::ContainerEq(avl_preorder));
}


TEST_F(TestPaVTAVL, AVLInsertRightLeftRot) {

  std::vector<int> avl_data{9, 12, 4, 17, 10, 11};
  std::vector<int> avl_preorder{max_val, 10, 9, 4, 12, 11, 17};
  std::vector<int> avl_inorder(avl_preorder.begin(), avl_preorder.end());
  std::sort(avl_inorder.begin(), avl_inorder.end());

  PaVTBST* avl_tree = new PaVTBST(true);
  for (int i=0; i<avl_data.size(); i++) {
    avl_tree->insert(avl_data[i]);
  }
  std::vector<int> inorder = inOrderTraversal(*avl_tree);
  std::vector<int> preorder = preOrderTraversal(*avl_tree);
  delete avl_tree;

  EXPECT_THAT(inorder, testing::ContainerEq(avl_inorder));
  EXPECT_THAT(preorder, testing::ContainerEq(avl_preorder));
}
}