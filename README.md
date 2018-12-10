# PaVT BST
## Introduction
**Pa**th **V**alidation in search **T**rees (PaVT) is a validation check that allows concurrent updates to *any* search-tree data structure by checking if a node being searched for has been relocated. The PaVT condition was proposed by Draschler-Cohen et al. in the paper *Practical Concurrent Traversals in Search Trees*[1]. This repo is a reimplimination of their work using the PaVT condidition on a Binary Search Tree data structure with and without an AVL balancing condition. 

There are two implementations of the BST data structure. One uses mutex locks for synchronization, `PaVTBST`, which is in the `PaVTBST.h` header file. The other is `STMPaVTBST` which is in the `STMPaVTBST.h` header file. This class uses GCC's Software Transactional Memory (STM) framework.

## Setup
+ This package requires C++11 or greater.
+ This package requires the use of a Linux machine.
### Compiling
+ PaVTBST
  - g++ -std=c++11 /path/to/PaVTBST.h /path/to/main -lpthread -o <nameofmain>
  
+ STMPaVTBST
  - g++ -std=c++11 -fgnu-tm /path/to/PaVTBST.h /path/to/main -lpthread -o <nameofmain>
## Interface
  
### Creating an Instance of a BST or AVL
```C++
PaVTBST *bst = new PaVTBST();
PaVTBST *avl = new PaVTBST(true);
```
  
### Class functions of data structure
```C++
void PaVTBST::insert(int &key);
void PaVTBST::remove(int &key);
bool PaVTBST::contains(int &key);
```

### Creating Instance of a BST or AVL with STM
```C++
STMPaVTBST *bst = new STMPaVTBST();
STMPaVTBST *avl = new STMPaVTBST(true);
```
## References
[1] [D. Drachsler-Cohen, M. Vechev, and E. Yahav. 2018. Practical concurrent traversals in search trees. In Proceedings of the 23rd ACM SIGPLAN Symposium on Principles and Practice of Parallel Programming (PPoPP '18). ACM, New York, NY, USA, 207-218.](https://files.sri.inf.ethz.ch/website/papers/ppopp18.pdf)
