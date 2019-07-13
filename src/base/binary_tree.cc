#include <stack>

#include <ttree/base/binary_tree.h>

namespace pavt {
namespace base {
namespace {

void update_parents(
    BinaryTree::Node* new_root,
    BinaryTree::Node* node,
    BinaryTree::Node* parent,
    BinaryTree::Node* subtree) {
  if(subtree != nullptr) {
    subtree->parent = node;
  }

  BinaryTree::Node *rootParent = parent;

  if (rootParent->right == node) {
    rootParent->right = new_root;
  } else {
    rootParent->left = new_root;
  }
  new_root->parent = rootParent;
  node->parent = new_root;
}
} // namespace

void BinaryTree::RotateLeft(Node *child, Node *node, Node *parent) {
  Node *newRoot = child;
  Node *temp = newRoot->left;
  node->right = temp;
  newRoot->left = node;

  update_parents(newRoot, node, parent, temp);
}

void BinaryTree::RotateRight(Node *child, Node *node, Node *parent) {
  Node* newRoot = child;
  Node *temp = newRoot->right;
  node->left = temp;
  newRoot->right = node;

  update_parents(newRoot, node, parent, temp);
}

class NodeDepth {
 public:
  BinaryTree::Node* node;
  int depth;
  NodeDepth(BinaryTree::Node *n, int d) {
    node=n;
    depth = d;
  }
};

std::vector<int> InOrderTraversal(BinaryTree &bst) {
  std::stack<BinaryTree::Node*> stack;

  std::vector<int> return_vals;
  BinaryTree::Node *curr = bst.GetRoot();

  while (!stack.empty() || curr!=nullptr) {
    if (curr!=nullptr) {
      stack.push(curr);
      curr = curr->left;
    } else {
      curr = stack.top();
      stack.pop();
      return_vals.push_back(curr->getKey());
      curr = curr->right;
    }
  }
  return return_vals;
}

std::vector<int> PreOrderTraversal(BinaryTree &bst) {

  std::stack<BinaryTree::Node*> stack;
  std::vector<int> return_vals;
  BinaryTree::Node *curr = bst.GetRoot();
  stack.push(curr);

  while (!stack.empty()) {

    curr=stack.top();
    return_vals.push_back(curr->getKey());
    stack.pop();

    if (curr->right) {
      stack.push(curr->right);
    }
    if (curr->left) {
      stack.push(curr->left);
    }
  }
  return return_vals;
}
} // namespace base
} // namespace pavt