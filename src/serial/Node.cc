//
// Created by tylertownsend on 9/20/18.
//

#include "../../include/Node.h"

Node::Node(int const& data) {
  setData(data);
  setLeft(nullptr);
  setRight(nullptr);
}

Node::~Node() {
  if (getLeft()!=nullptr) delete getLeft();
  if (getRight()!=nullptr) delete getRight();
}

int Node::getData() {
  return data;
}

void Node::setData(int const& data) {
  this->data = data;
}

Node* Node::getLeft() {
  return left;
}

Node* Node::getRight() {
  return right;
}

void Node::setLeft(Node* node) {
  this->left = node;
}

void Node::setRight(Node* node) {
  this->right = node;
}
