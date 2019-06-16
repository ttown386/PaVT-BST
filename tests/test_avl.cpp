#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "UtilitiesBST.h"
#include "PaVT/bst.h"
#include "PaVT/avl.h"

using namespace pavt;
namespace {

class TestPaVTAVL : public testing::Test {
 protected:
  const int max_val = std::numeric_limits<int>::max();
  AVL* avl;
  AVL* small_avl;
  const std::vector<int> dfs_balanced {max_val, 40, 20, 60, 10, 30, 50, 70};
  const std::vector<int> inorder_balanced {10, 20, 30, 40, 50, 60, 70, max_val};
  const std::vector<int> preorder_small {max_val, 4, 10};
  const std::vector<int> inorder_small {4, 10, max_val};

 public:
  void SetUp() {
    avl = init_balanced_avl();
    small_avl = init_small_tree();
  }

  void TearDown() {
    delete avl;
    delete small_avl;
  }
  
  void insert_vector_into_tree(AVL* tree, 
                               const std::vector<int>& node_list) {
    for (std::size_t i=0; i < node_list.size(); i++) {
      tree->insert(node_list[i]);
    }
  }

 private:
  AVL* init_balanced_avl() {
    AVL* bst = new AVL(true);
    insert_vector_into_tree(bst, dfs_balanced);
    return bst;
  }

  AVL* init_small_tree() {
    AVL* bst = new AVL();
    insert_vector_into_tree(bst, preorder_small);
    return bst;
  }
};


TEST_F(TestPaVTAVL, InsertSingleRightRotation) {
  int new_val = 14;
  const std::vector<int> new_preorder = {max_val, 10, 4, new_val};
  const std::vector<int> new_inorder = {4, 10, new_val, max_val}; 
  small_avl->insert(new_val);
  std::vector<int> inorder = inOrderTraversal(*small_avl);
  std::vector<int> preorder = preOrderTraversal(*small_avl);
  EXPECT_THAT(inorder, testing::ContainerEq(new_inorder));
  EXPECT_THAT(preorder, testing::ContainerEq(new_preorder));
}


TEST_F(TestPaVTAVL, InsertSingleLeftRotation) {
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

  AVL* avl_tree = new AVL(true);
  for (std::size_t i=0; i<avl_data.size(); i++) {
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

  AVL* avl_tree = new AVL(true);
  for (auto key = avl_data.begin(); key != avl_data.end(); key++) {
    avl_tree->insert(*key);
  }
  std::vector<int> inorder = inOrderTraversal(*avl_tree);
  std::vector<int> preorder = preOrderTraversal(*avl_tree);
  delete avl_tree;

  EXPECT_THAT(inorder, testing::ContainerEq(avl_inorder));
  EXPECT_THAT(preorder, testing::ContainerEq(avl_preorder));
}


TEST_F(TestPaVTAVL, AVLDeleteLogNRotations) {
  const std::vector<int> keys_to_minimal_avl {
    5, 15, 25, 35, 45, 55, 65, 75, 37, 57, 67, 72, 77, 80
  };

  for (auto key=keys_to_minimal_avl.begin();
      key!=keys_to_minimal_avl.end(); key++) {
    avl->insert(*key);
  }

  avl->remove(40);

  std::vector<int> avl_preorder = {
    max_val, 45, 20, 10, 5, 15, 30, 25, 35, 37, 60,
    50, 55, 57, 70, 65, 67, 75, 72, 77, 80
  };
  std::vector<int> preorder = preOrderTraversal(*avl);
  EXPECT_THAT(preorder, testing::ContainerEq(avl_preorder));
}
}