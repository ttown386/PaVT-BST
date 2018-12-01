* The program was written in C++ and requires:
	- C++11
* make sure to run in a unix-like operating system. It requires the use of the posix library for pthreads. It was run in an ubuntu environment provided by windows App store.

==================================================
To compile from command line (Linux)
	1. cd into directory
	2. Type "g++ -std=c++11 -fgnu-tm SerialStmBst.cc Node.cc -lpthread"
To run
	1. type "./a.out [AVL] [TOTALOPS] [ADD] [REMOVE] [CONTAINS] [SIZE]
	2. AVL = 0 for BST and 1 for AVL
	3. TOTALOPS = number of total operations by all threads
	4. ADD = integer (0, 100) for percentage of insert operations
	5. REMOVE = integer (0, 100) for percentage of remove operations
	6. CONTAINS = integer (0, 100) for percentage of contain operations
	7. SIZE = integer (1,2) for transaction size
Note:
	ADD + REMOVE + CONTAINS = 100. There is no guard against it so this condition must be met by the user to prevent undefined behavior

==================================================
Note:
	This implementation is can be used to create either an AVL tree or a BST tree. This concurrent data structure uses atomic transactions that use an all or nothing system where it either commits its work or rollsback depending on whether another transaction has changed some values around.

==================================================
To create the six out of nine benchmarks for this STM, run the following after compiling:
	For 9%Insert/1%Remove/90%Contains
		For BST with transaction size of 1:
			./a.out 0 2000000 9 1 90 1
		For AVL with transaction size of 1:
			./a.out 1 2000000 9 1 90 1
		For BST with transaction size of 2:
			./a.out 0 2000000 9 1 90 2
		For AVL with transaction size of 2:
			./a.out 1 2000000 9 1 90 2
	For 20%Insert/10%Remove/70%Contains
		For BST with transaction size of 1:
			./a.out 0 2000000 20 10 70 1
		For AVL with transaction size of 1:
			./a.out 1 2000000 20 10 70 1
		For BST with transaction size of 2:
			./a.out 0 2000000 20 10 70 2
		For AVL with transaction size of 2:
			./a.out 1 2000000 20 10 70 2
	For 50%Insert/50%Remove/0%Contains
		For BST with transaction size of 1:
			./a.out 0 2000000 50 50 0 1
		For AVL with transaction size of 1:
			./a.out 1 2000000 50 50 0 1
		For BST with transaction size of 2:
			./a.out 0 2000000 50 50 0 2
		For AVL with transaction size of 2:
			./a.out 1 2000000 50 50 0 2