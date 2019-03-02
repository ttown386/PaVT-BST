
//
// Created by ttown on 9/30/2018.
//

#ifndef CONCURRENTTREETRAVERALS_STMBST_H
#define CONCURRENTTREETRAVERALS_STMBST_H

class STMNode {
 private:
  int key;
  STMNode *left;
  STMNode *right;
  STMNode *parent;
  int height;
 public:
  bool mark;
  STMNode* leftSnap;
  STMNode* rightSnap;
  STMNode(int const& key) transaction_safe {
    setKey(key);
    setLeft(nullptr);
    setRight(nullptr);
    setParent(nullptr);
    leftSnap = rightSnap = this;
    setHeight(0);
    mark = false;
  }

  ~STMNode() transaction_safe {
    if (getLeft()!=nullptr) delete getLeft();
    if (getRight()!=nullptr) delete getRight();
  }

  int getKey(){
    return key;
  }

  STMNode *get(int field) transaction_safe {
    if (field==0) return this->getLeft();
    if (field==1) return this->getRight();
    if (field==2) return this;
  }

  void setKey(int const& key) transaction_safe {
    this->key = key;
  }

  int getHeight() transaction_safe {
    return height;
  }

  void setHeight(int const& height) transaction_safe {
    this->height = height;
  }

  STMNode *getLeft() transaction_safe {
    return left;
  }

  STMNode *getRight() transaction_safe {
    return right;
  }

  STMNode *getParent() transaction_safe {
    return parent;
  }

  void setLeft(STMNode *node) transaction_safe {
    this->left = node;
  }

  void setRight(STMNode *node) transaction_safe {
    this->right = node;
  }

  void setParent(STMNode *node) transaction_safe {
    this->parent = node;
  }
};

class STMPaVTBST {
 private:
  bool isAvl;
  STMNode *root=nullptr;
  STMNode *maxSentinel;
  STMNode *minSentinel;
  int nextField(STMNode *node, int const &key);
  void rotateLeft(STMNode *child, STMNode *node, STMNode *parent);
  void rotateRight(STMNode *child, STMNode *node, STMNode *parent);
  int height(STMNode *node);
  void rebalance(STMNode *node);
  STMNode *traverse(STMNode *node, int const &key);

 public:
	STMPaVTBST(bool isAvl=false);
  ~STMPaVTBST();
  void insert(int const &key);
  void remove(int const &key);
  bool contains(int const &key);
  STMNode *getRoot();
  STMNode *getMinSentinel();
  STMNode *getMaxSentinel();
};

#endif //CONCURRENTTREETRAVERALS_STMBST_H
