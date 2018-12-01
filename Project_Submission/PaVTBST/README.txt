* The program was written in C++ and requires:
	- C++11
* make sure to run in a unix-like operating system. It requires the use of the posix library for pthreads. It was run in an ubuntu environment provided by windows App store.

==================================================
To compile from command line (Linux)
	1. cd into directory
	2. Type "g++ -std=c++11 SerialStmBst.cc Node.cc -lpthread"
To run
	1. type "./a.out [AVL] [TOTALOPS] [ADD] [REMOVE] [CONTAINS]
	2. AVL = 0 for BST and 1 for AVL
	3. TOTALOPS = number of total operations by all threads
	4. ADD = integer (0, 100) for percentage of insert operations
	5. REMOVE = integer (0, 100) for percentage of remove operations
	6. CONTAINS = integer (0, 100) for percentage of contain operations
Note:
	ADD + REMOVE + CONTAINS = 100. There is no guard against it so this condition must be met by the user to prevent undefined behavior

==================================================
Note:
	This implementation is can be used to create either an AVL tree or a BST tree. This concurrent data structure can be used for a lock-free contains and a deadlock-free remove and insert. 