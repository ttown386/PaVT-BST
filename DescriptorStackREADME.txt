 * The assigment was written in C++ and requires: 
	- C++11 
 * make sure to run in a unix-like operating system. It requires
   the use of the posix library for pthreads. It was run in an 
   ubuntu environment provided by windows App store.

============================================================
To compile from command line (Linux)
	1. cd into directory
	2. Type "g++ -std=c++11 DescriptorStack.cc -lpthread"
To Run
	1. type "./a.out [THREADS] [TOTALOPS] [PUSH] [POP] [SIZE]"
	2. THREADS = number of threads
	3. TOTALOPS = number of total operations done by all threads
	4. PUSH = integer (0, 100) for percentage of push and pop operations
	5. POP = integer (0, 100) for percentage of pop operations
	6. SIZE = integer (0, 100) for percentage of size operations
Note: 
	PUSH + POP + SIZE = 100. There is no guard against it so this condition must
	be met by user to prevent undefined behavior

============================================================
Note:
	This program creates new descriptor objects through each iteration of a while
	loop and to prevent polluting results with overhead of memory management, 
	memory leaks are let to run wild. Be cautious with low memory systems.
