#include <stack>

#include <PaVT/Base/binary_tree.h>

namespace pavt {
namespace base {

class NodeDepth {
 public:
  BinaryTree::Node* node;
  int depth;
  NodeDepth(BinaryTree::Node *n, int d) {
    node=n;
    depth = d;
  }
};

std::vector<int> inOrderTraversal(BinaryTree &bst) {
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

std::vector<int> preOrderTraversal(BinaryTree &bst) {

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