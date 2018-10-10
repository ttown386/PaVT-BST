//
// Created by ttown on 10/6/2018.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <list>
#include "SerialUtils.h"

#include "BinarySearchTree.h"

namespace {
class TestBinarySearchTree : public testing::Test {
 protected:
  Node *root, *node1, *node2, *node3;
  BinarySearchTree bst;
  BinarySearchTree avl;
  const int numRange = 10;
  std::list<int> listRange;
  const int balanced[7] =  {4, 2, 6, 1, 3, 5, 7};

 public:
  virtual void SetUp();
  virtual void TearDown();
};
}

void TestBinarySearchTree::SetUp() {
  bst = BinarySearchTree();
  avl = BinarySearchTree(true);
  for (int i=0; i<numRange; i++) {
    listRange.push_back(i);
  }
  node1 = new Node(1);
  node2 = new Node(2);
  node3 = new Node(3);
  root = new Node(4);
}

void TestBinarySearchTree::TearDown() {
  ;
}

TEST_F(TestBinarySearchTree, BSTInsert) {

  // Initialize linked list bst
  for (int i=0; i<numRange; i++) {
    bst.insert(i);
  }

  bool areEqual = true;
  std::list<int> inOrder = inOrderTraversal(bst);
  std::list<int>::iterator itInOrder=inOrder.begin();
  std::list<int>::iterator itListRange=listRange.begin();
  for (int i=0; i<inOrder.size(); i++) {
    areEqual &= (*itInOrder == *itListRange);
    itInOrder++;
    itListRange++;
  }

  ASSERT_EQ(areEqual, true);
}

TEST_F(TestBinarySearchTree, Height) {

  // Initail height
  ASSERT_EQ(root->getHeight(), 0);

  root->setLeft(node1);
  node1->setParent(root);
  bst.updateHeights(node1);
  ASSERT_EQ(bst.height(root), 1);

  root->setRight(node2);
  node2->setParent(root);
  bst.updateHeights(node2);
  ASSERT_EQ(bst.height(root), 1);

  root->getRight()->setRight(node3);
  node3->setParent(root->getRight());
  bst.updateHeights(node3);
  ASSERT_EQ(bst.height(root), 2);

  // try a null ptr
  ASSERT_EQ(bst.height(node3->getLeft()), -1);
}

TEST_F(TestBinarySearchTree, BalanceFactor) {

  // Initail height
  ASSERT_EQ(bst.balanceFactor(root), 0);

  root->setLeft(node1);
  node1->setParent(root);
  bst.updateHeights(node1);
  ASSERT_EQ(bst.balanceFactor(root), 1);

  root->setRight(node2);
  node2->setParent(root);
  bst.updateHeights(node2);
  ASSERT_EQ(bst.balanceFactor(root), 0);

  root->getRight()->setRight(node3);
  node3->setParent(root->getRight());
  bst.updateHeights(node3);
  ASSERT_EQ(bst.balanceFactor(root), -1);

  Node* temp = new Node(7);
  root->getRight()->getRight()->setRight(temp);
  temp->setParent(root->getRight()->getRight());
  bst.updateHeights(temp);
  ASSERT_EQ(bst.balanceFactor(root), -2);
  delete temp;

}


TEST_F(TestBinarySearchTree, RotateLeft) {

}

TEST_F(TestBinarySearchTree, RotateRight) {

}

TEST_F(TestBinarySearchTree, ReBalance) {

}

TEST_F(TestBinarySearchTree, AVLInsertSingleRotLeft) {

  // initialize avl
  for (int i=1; i<=numRange; i++) {
    avl.insert(i);
  }

  bool areEqual = true;
  std::list<int> inOrder = inOrderTraversal(avl);
  std::list<int>::iterator it = inOrder.begin();
  for (int i=1; i<=inOrder.size(); i++) {
    int temp = *it++;
//    std::cout<<temp<<" \n";
    areEqual &= (temp == i);
  }

  ASSERT_EQ(areEqual, true);
}

TEST_F(TestBinarySearchTree, AVLInsertSingleRotRight) {

  // initialize avl
  for (int i=numRange; i>0; i--) {
    avl.insert(i);
  }

  bool areEqual = true;
  std::list<int> inOrder = inOrderTraversal(avl);
  std::list<int>::iterator it = inOrder.begin();
  for (int i=1; i<=inOrder.size(); i++) {
    int temp = *it++;
//    std::cout<<temp<<" \n";
    areEqual &= (temp == i);
  }

  ASSERT_EQ(areEqual, true);
}

TEST_F(TestBinarySearchTree, AVLInsertLeftRightRot) {

  // initialize avl
  int avlData[] {12, 2, 18, 1, 10, 21, 11, 7, 9};
  int n = sizeof(avlData) / sizeof(avlData[0]);
  std::vector<int> dest(avlData, avlData+n);
  for (int i=0; i<dest.size(); i++) {
    avl.insert(dest.at(i));
  }

  // sort the vector
  std::sort(dest.begin(), dest.end());

  // in order traversal
  bool areEqual = true;

  std::list<int> inOrder = inOrderTraversal(avl);
  std::list<int>::iterator it = inOrder.begin();
  for (int i=0; i<inOrder.size(); i++) {
    int temp = *it++;
//    std::cout<<temp<<" \n";
    areEqual &= (temp == dest.at(i));
  }

  int avlDataPre[] {10, 2, 1, 7, 9, 12, 11, 18, 21};
  std::list<int> preOrder = preOrderTraversal(avl);
  it = preOrder.begin();
  for (int i=0; i<preOrder.size(); i++) {
    int temp = *(it++);
    areEqual &= (temp == avlDataPre[i]);
  }
  ASSERT_EQ(areEqual, true);
}

TEST_F(TestBinarySearchTree, AVLInsertRightLeftRot) {

  // initialize avl
  int avlData[] {9, 12, 4, 17, 10, 11};
  int n = sizeof(avlData) / sizeof(avlData[0]);
  std::vector<int> dest(avlData, avlData+n);
  for (int i=0; i<dest.size(); i++) {
    avl.insert(dest.at(i));
  }

  // sort the vector
  std::sort(dest.begin(), dest.end());

  bool areEqual = true;
  // Try in order traversal
  std::list<int> inOrder = inOrderTraversal(avl);
  std::list<int>::iterator it = inOrder.begin();
  for (int i=0; i<inOrder.size(); i++) {
    int temp = *it++;
//    std::cout<<temp<<" \n";
    areEqual &= (temp == dest.at(i));
  }
  // Now try preOrder traversal
  int avlDataPre[] {10, 9, 4, 12, 11, 17};
  std::list<int> preOrder = preOrderTraversal(avl);
  it = preOrder.begin();
  for (int i=0; i<preOrder.size(); i++) {
    int temp = *(it++);
    areEqual &= (temp == avlDataPre[i]);
  }
  ASSERT_EQ(areEqual, true);
}
