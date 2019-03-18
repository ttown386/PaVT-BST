//
// Created by ttown on 10/6/2018.
//

#include <gtest/gtest.h>
#include <iostream>
#include <list>

#include "UtilitiesBST.h"
#include "PaVT/PaVTBST.h"

namespace {
class TestPaVTBST : public testing::Test {
 protected:
  Node *root, *node1, *node2, *node3;
  PaVTBST *bst;
  PaVTBST *avl;
  const int numRange = 10;
  std::list<int> listRange;
  const int balanced[7] =  {4, 2, 6, 1, 3, 5, 7};

 public:
  virtual void SetUp();
  virtual void TearDown();
};
}

void TestPaVTBST::SetUp() {
  bst = new PaVTBST();
  avl = new PaVTBST(true);
  for (int i=0; i<numRange; i++) {
    listRange.push_back(i);
  }
  listRange.push_back(std::numeric_limits<int>::max());
  node1 = new Node(1);
  node2 = new Node(2);
  node3 = new Node(3);
  root = new Node(4);
}

void TestPaVTBST::TearDown() {
  delete bst;
  delete avl;
}

TEST_F(TestPaVTBST, BSTInsert) {

  for (int i=0; i<numRange; i++) {
    bst->insert(i);
  }


  bool areEqual = true;
  std::list<int> inOrder = inOrderTraversal(*bst);
  std::list<int>::iterator itInOrder=inOrder.begin();
  std::list<int>::iterator itListRange=listRange.begin();
  for (int i=0; i<inOrder.size(); i++) {
    areEqual &= (*itInOrder == *itListRange);
    itInOrder++;
    itListRange++;
  }
  ASSERT_EQ(areEqual, true);
}

// TEST_F(TestPaVTBST, RotateLeft) {

// }

// TEST_F(TestPaVTBST, RotateRight) {

// }

// TEST_F(TestPaVTBST, ReBalance) {

// }

// TEST_F(TestPaVTBST, AVLInsertSingleRotLeft) {

//   // initialize avl
//   for (int i=1; i<=numRange; i++) {
//     avl.insert(i);
//   }

//   bool areEqual = true;
//   std::list<int> inOrder = inOrderTraversal(avl);
//   std::list<int>::iterator it = inOrder.begin();
//   for (int i=1; i<=inOrder.size(); i++) {
//     int temp = *it++;
// //    std::cout<<temp<<" \n";
//     areEqual &= (temp == i);
//   }

//   ASSERT_EQ(areEqual, true);
// }

// TEST_F(TestPaVTBST, AVLInsertSingleRotRight) {

//   // initialize avl
//   for (int i=numRange; i>0; i--) {
//     avl.insert(i);
//   }

//   bool areEqual = true;
//   std::list<int> inOrder = inOrderTraversal(avl);
//   std::list<int>::iterator it = inOrder.begin();
//   for (int i=1; i<=inOrder.size(); i++) {
//     int temp = *it++;
// //    std::cout<<temp<<" \n";
//     areEqual &= (temp == i);
//   }

//   ASSERT_EQ(areEqual, true);
// }

// TEST_F(TestPaVTBST, AVLInsertLeftRightRot) {

//   // initialize avl
//   int avlData[] {12, 2, 18, 1, 10, 21, 11, 7, 9};
//   int n = sizeof(avlData) / sizeof(avlData[0]);
//   std::vector<int> dest(avlData, avlData+n);
//   for (int i=0; i<dest.size(); i++) {
//     avl.insert(dest.at(i));
//   }

//   // sort the vector
//   std::sort(dest.begin(), dest.end());

//   // in order traversal
//   bool areEqual = true;

//   std::list<int> inOrder = inOrderTraversal(avl);
//   std::list<int>::iterator it = inOrder.begin();
//   for (int i=0; i<inOrder.size(); i++) {
//     int temp = *it++;
// //    std::cout<<temp<<" \n";
//     areEqual &= (temp == dest.at(i));
//   }

//   int avlDataPre[] {10, 2, 1, 7, 9, 12, 11, 18, 21};
//   std::list<int> preOrder = preOrderTraversal(avl);
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
//     avl.insert(dest.at(i));
//   }

//   // sort the vector
//   std::sort(dest.begin(), dest.end());

//   bool areEqual = true;
//   // Try in order traversal
//   std::list<int> inOrder = inOrderTraversal(avl);
//   std::list<int>::iterator it = inOrder.begin();
//   for (int i=0; i<inOrder.size(); i++) {
//     int temp = *it++;
// //    std::cout<<temp<<" \n";
//     areEqual &= (temp == dest.at(i));
//   }
//   // Now try preOrder traversal
//   int avlDataPre[] {10, 9, 4, 12, 11, 17};
//   std::list<int> preOrder = preOrderTraversal(avl);
//   it = preOrder.begin();
//   for (int i=0; i<preOrder.size(); i++) {
//     int temp = *(it++);
//     areEqual &= (temp == avlDataPre[i]);
//   }
//   ASSERT_EQ(areEqual, true);
// }
