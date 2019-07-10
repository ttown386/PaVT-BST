#ifndef TTREE_INCLUDE_PAVT_SEQUENTIAL_BST_H_
#define TTREE_INCLUDE_PAVT_SEQUENTIAL_BST_H_ 

#include <PaVT/Base/binary_tree.h>

namespace pavt {
namespace seq {

class BST : public base::BinaryTree {
 public:
  BST();
  ~BST();
  void Insert(const int& key);
  void Remove(const int& key);
  bool Contains(const int& key);

 protected:
  Node* Root() { return root; }
  Node* Insert(Node* node);
  std::pair<Node*, Node*>* Remove(Node* node, const int& key); 
  Node* Traverse(Node* node, const int& key);
  Node* Contains(Node* node, const int& key);
};
}
}
#endif // TTREE_INCLUDE_PAVT_SEQUENTIAL_BST_H_ 
