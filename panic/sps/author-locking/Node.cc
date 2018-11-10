//
// Created by tylertownsend on 9/20/18.
//

#include "Node.h"

Node::Node(int const& data) {
  setData(data);
  setLeft(nullptr);
  setRight(nullptr);
  setParent(nullptr);
  succ = pred = this;
  setHeight(0);
  mark = false;
  sentinel = false;
}

Node::~Node() {
//  if (getLeft()!=nullptr) delete getLeft();
//  if (getRight()!=nullptr) delete getRight();
}

int Node::getData() {
  return data;
}

Node *Node::get(int field) {
  if (field==0) return this->getLeft();
  if (field==1) return this->getRight();
  if (field==2) return this;
}

void Node::setData(int const& data) {
  this->data = data;
}

int Node::getHeight() {
  return height;
}

void Node::setHeight(int const& height) {
  this->height = height;
}

Node* Node::getLeft() {
  return left;
}

Node* Node::getRight() {
  return right;
}

Node* Node::getParent() {
  return parent;
}

void Node::setLeft(Node* node) {
  this->left = node;
}

void Node::setRight(Node* node) {
  this->right = node;
}

void Node::setParent(Node* node) {
  this->parent = node;
}
