//
// Created by bbwor on 9/24/2018.
//

#include "../../include/ExternalBST.h"
#include <algorithm>

ExternalBST::InternalBST() {
  root = nullptr;
}

ExternalBST::~InternalBST() {
  delete root;
}

/*!
 * Inserts a created node and returns whether or not it was successful
 * Allows for duplicate nodes
 *
 * @param The key is the data to be inserted into the tree
 * @return Returns true if we successfully inserted | Returns false if it was already in the list
 */
bool ExternalBST::insert(int const &key) {

  //Checking if root even exists
  if(root == nullptr) {
    root = new Node(key);
    return true;
  }

  //Iterating through the tree until we find an appropriate spot
  Node *temp = root;
  while(1) {
    //Our key is already in the data set!
    if(key == temp->getData()){
      Node *node = new Node(key);
      node->setLeft(temp->getLeft());
      temp->setLeft(node);
      return true;
    }

    //If it is greater then the data then take the right path until we reach a null pointer
    // Then we insert a new node with key as the data
    if(key > temp->getData()){
      if(temp->getRight() == nullptr){
        temp->setRight(new Node(key));
        return true;
      } else{
        temp = temp->getRight();
      }
    }
      //Otherwise go left until we hit a null pointer and once we do we set the left path to be a new node
      // Then we insert a new node with key as the data
    else{
      if(temp->getLeft() == nullptr){
        temp->setLeft(new Node(key));
        return true;
      } else{
        temp = temp->getLeft();
      }
    }
  }
}

/*!
 * Removes an item from the BST based on the key
 * Doesn't balance tree but at least grabs the children from the parent
 *
 * @param key is the data we wish to find and remove
 * @return false if we didn't find the item otherwise it returns the true
 */
bool ExternalBST::remove(int const& key){

  if(root == nullptr){
    return false;
  }

  //This means that root needs to be removed
  if(key == root->getData()) {
    //Means there is a right child
    if(root->getRight() != nullptr){
      Node* minRoot = findMin(root->getRight());
      root->setData(minRoot->getData());
      //Our right node is the minimum so make sure not to trim off its children when we delete
      if(root->getRight() == minRoot){
        root->setRight(root->getRight()->getRight());
      }
      delete minRoot;
      return true;
    } else if(root->getLeft() != nullptr){
      //If true then there is only a left child
      Node *temp = root->getLeft();
      root->setData(root->getLeft()->getData());
      root->setRight(root->getLeft()->getRight());
      root->setLeft(root->getLeft()->getLeft());
      delete temp;
      return true;
    }else{
      //Delete since only the root is left
      delete root;
      return true;
    }
  }

  //Iterating throughout BST for item to remove
  Node *temp = root;
  while(temp != nullptr) {

    //If it is greater then the data then take the right path until we reach a null pointer
    //Then we insert a new node with key as the data
    if(key > temp->getData()){
      Node *rightNode = temp->getRight();
      if(rightNode == nullptr){
        return false;
      } else if(rightNode->getData() == key){
        if(rightNode->getRight() != nullptr && rightNode->getLeft() != nullptr){
          //Executing because the right node has two children
          if(rightNode->getRight() != nullptr){
            Node* minRoot = findMin(rightNode->getRight());
            rightNode->setData(minRoot->getData());
            //Our right node is the minimum so make sure not to trim off its children when we delete
            if(rightNode->getRight() == minRoot){
              rightNode->setRight(rightNode->getRight()->getRight());
            }
            delete minRoot;
            return true;
          }
        } else if(rightNode->getRight() != nullptr){
          //Means we only have a right child
          temp->setRight(rightNode->getRight());
        } else{
          //Means we only have a left child
          temp->setRight(rightNode->getLeft());
        }
      } else {
        temp = rightNode;
      }
      //Otherwise go left until we hit a null pointer and once we do we set the left path to be a new node
      //Then we insert a new node with key as the data
    } else if(key < temp->getData()){
      Node *leftNode = temp->getLeft();
      if(leftNode == nullptr){
        return false;
      } else if(leftNode->getData() == key){
        if(leftNode->getRight() != nullptr && leftNode->getLeft() != nullptr){
          //Executing because the right node has two children
          if(leftNode->getRight() != nullptr){
            Node* minRoot = findMin(leftNode->getRight());
            leftNode->setData(minRoot->getData());
            //Our right node is the minimum so make sure not to trim off its children when we delete
            if(leftNode->getRight() == minRoot){
              leftNode->setRight(leftNode->getRight()->getRight());
            }
            delete minRoot;
            return true;
          }
        } else if(leftNode->getRight() != nullptr){
          //Means we only have a right child
          temp->setRight(leftNode->getRight());
        } else{
          //Means we only have a left child
          temp->setLeft(leftNode->getLeft());
        }
      } else {
        temp = leftNode;
      }
    }
  }

  return false;
}

/*!
 * @param The key to find in the tree
 * @return Boolean that is true if key is found and false otherwise
 */
bool ExternalBST::contains(int const& key){

  Node *temp = root;
  while(temp != nullptr){
    if(temp->getData() == key){
      return true;
    } else if (key > temp->getData()){
      temp = temp->getRight();
    } else {
      temp = temp->getLeft();
    }
  }

  return false;
}

/*!
 *
 * @param Expecting the right node of a node to be removed with two children
 * @return Lowest valued node
 */
Node* InternalBST::findMin(Node *node) {
  if(node->getLeft() == nullptr) {
    node = node->getLeft();
  }
  while(node->getLeft()->getLeft() != nullptr) {
    node = node->getLeft();
  }
  //If the node we are returning has items attached to it make sure to not trim those off
  //We do so by setting our left node to their right node
  if(node->getLeft()->getRight() != nullptr) {
    node->setLeft(node->getLeft()->getRight());
  }
  return node->getLeft();
}
