#include <iostream>
#include <stdlib.h>

#include <PaVT/pavt/avl.h>

namespace pavt {

void AVL::Insert(const int& key) {
  Node* new_node = new Node(key);
  auto return_node = (Node*)PaVTBST::Insert(new_node);
  if (return_node == nullptr) {
    delete new_node;
  } else {
    Rebalance(return_node);
  }
}

void AVL::Remove(const int& key) {
  auto balance_nodes = PaVTBST::Remove((Node*)root, key);
  if (balance_nodes->first != nullptr) {
    Rebalance((Node*)balance_nodes->first);
    if (balance_nodes->second != nullptr) Rebalance((Node*)balance_nodes->second);
  } 
  delete balance_nodes;
}

bool AVL::Contains(const int& key) {
  return PaVTBST::Contains((Node*)root, key);
}

// Rotates node to the left. Child becomes nodes parent.
void AVL::RotateLeft(Node *child, Node *node, Node *parent) {

  base::BinaryTree::RotateLeft(child, node, parent); 

  // Update the tree heights
  int leftHeight = Height((Node*)node->left);
  int rightHeight = Height((Node*)node->right);
  node->height = (1 + std::max(leftHeight, rightHeight));

  int newRootLeftHeight = Height((Node*)child->left);
  int newRootRightHeight = Height((Node*)child->right);
  child->height = (1 + std::max(newRootLeftHeight, newRootRightHeight));
}

//Rotates node to the right. Child becomes nodes parent
void AVL::RotateRight(Node *child, Node *node, Node *parent) {

  base::BinaryTree::RotateRight(child, node, parent);
  // Update the tree heights
  int leftHeight = Height((Node*)node->left);
  int rightHeight = Height((Node*)node->right);
  node->height = (1 + std::max(leftHeight, rightHeight));

  int newRootLeftHeight = Height((Node*)child->left);
  int newRootRightHeight = Height((Node*)child->right);
  child->height = (1 + std::max(newRootLeftHeight, newRootRightHeight));
}

/*
 * Returns the height of node
 */
int AVL::Height(Node *node) {
  return (node == nullptr) ? -1 : node->height;
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 */
void AVL::Rebalance(Node *node) {

  if (node==root) {
    return;
  }

  Node *parent = (Node*)node->parent;

  while(node!=root) {

    // lock parent
    parent->lock.lock();
    if (node->parent!=parent) {
      parent->lock.unlock();
      if (node->IsMarked()) {
        return;
      }

      parent = (Node*)node->parent;
      continue;
    }

    // lock node
    node->lock.lock();
    if (node->IsMarked()) {
      node->lock.unlock();
      parent->lock.unlock();
      return;
    }

    Node *left = (Node*)node->left;
    Node *right= (Node*)node->right;

    int leftHeight = Height(left);
    int rightHeight = Height(right);

    int currHeight = std::max(leftHeight, rightHeight) + 1;
    int prevHeight = node->height;

    int bf = leftHeight - rightHeight;
    if (currHeight != prevHeight) {
      node->height = (currHeight);
    } else if (bf <= 1) {
      node->lock.unlock();
      parent->lock.unlock();
      return;
    }

    Node *child;
    // The node's right subtree is too tall
    if (bf < MINBF) {
      child = right;
      child->lock.lock();

      Node *childLeft = (Node*)child->left;
      Node *childRight = (Node*)child->right;

      int childLeftHeight = Height(childLeft);
      int childRightHeight = Height(childRight);

      int childBf = childLeftHeight - childRightHeight;

      Node *grandChild = childLeft;

      // Need to do double rotation
      if (childBf > 0) {
        grandChild->lock.lock();
        RotateRight(grandChild, child, node);
        RotateLeft(grandChild, node, parent);
        child->lock.unlock();
        node->lock.unlock();
        grandChild->lock.unlock();
        parent->lock.unlock();

        node = grandChild;

      } else {
        RotateLeft(child, node, parent);
        node->lock.unlock();
        child->lock.unlock();
        parent->lock.unlock();

        node = child;
      }

      // The node's left subtree is too tall
    } else if (bf > MAXBF) {
      child = left;
      child->lock.lock();

      Node *childLeft = (Node*)child->left;
      Node *childRight = (Node*)child->right;

      int childLeftHeight = Height(childLeft);
      int childRightHeight = Height(childRight);

      int childBf = childLeftHeight - childRightHeight;

      Node *grandChild = childRight;

      if (childBf < 0) {
        grandChild->lock.lock();

        RotateLeft(grandChild, child, node);
        RotateRight(grandChild, node, parent);
        node->lock.unlock();
        child->lock.unlock();
        grandChild->lock.unlock();
        parent->lock.unlock();

        node = grandChild;
      } else {

        RotateRight(child, node, parent);

        node->lock.unlock();
        child->lock.unlock();
        parent->lock.unlock();

        node = child;
      }

    } else {

      node->lock.unlock();
      parent->lock.unlock();

      // Traverse back up tree
      node = parent;
      parent = (Node*)node->parent;
    }
  }
}
} // namespace pavt