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

// TEST_F(TestPaVTBST, AVLInsertLeftRightRot) {

//   // initialize avl
//   int avlData[] {12, 2, 18, 1, 10, 21, 11, 7, 9};
//   int n = sizeof(avlData) / sizeof(avlData[0]);
//   std::vector<int> dest(avlData, avlData+n);
//   for (int i=0; i<dest.size(); i++) {
//     avl->insert(dest.at(i));
//   }

//   // sort the vector
//   std::sort(dest.begin(), dest.end());

//   // in order traversal
//   bool areEqual = true;

//   std::list<int> inOrder = inOrderTraversal(*avl);
//   std::list<int>::iterator it = inOrder.begin();
//   for (int i=0; i<inOrder.size(); i++) {
//     int temp = *it++;
// //    std::cout<<temp<<" \n";
//     areEqual &= (temp == dest.at(i));
//   }

//   int avlDataPre[] {10, 2, 1, 7, 9, 12, 11, 18, 21};
//   std::list<int> preOrder = preOrderTraversal(*avl);
//   it = preOrder.begin();
//   for (int i=0; i<preOrder.size(); i++) {
//     int temp = *(it++);
//     areEqual &= (temp == avlDataPre[i]);
//   }
//   ASSERT_EQ(areEqual, true);
// }

// TEST_F(TestPaVTBST, AVLInsertRightLeftRot) {

//   // initialize avl
//   int avlData[] {9, 12, 4, 17, 10, 11};
//   int n = sizeof(avlData) / sizeof(avlData[0]);
//   std::vector<int> dest(avlData, avlData+n);
//   for (int i=0; i<dest.size(); i++) {
//     avl->insert(dest.at(i));
//   }

//   // sort the vector
//   std::sort(dest.begin(), dest.end());

//   bool areEqual = true;
//   // Try in order traversal
//   std::list<int> inOrder = inOrderTraversal(*avl);
//   std::list<int>::iterator it = inOrder.begin();
//   for (int i=0; i<inOrder.size(); i++) {
//     int temp = *it++;
// //    std::cout<<temp<<" \n";
//     areEqual &= (temp == dest.at(i));
//   }
//   // Now try preOrder traversal
//   int avlDataPre[] {10, 9, 4, 12, 11, 17};
//   std::list<int> preOrder = preOrderTraversal(*avl);
//   it = preOrder.begin();
//   for (int i=0; i<preOrder.size(); i++) {
//     int temp = *(it++);
//     areEqual &= (temp == avlDataPre[i]);
//   }
//   ASSERT_EQ(areEqual, true);
// }
}