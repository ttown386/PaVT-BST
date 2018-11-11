#include <mutex>
#include <iostream>
#include <thread>
#include <string>
#include <atomic>

//Seems marking structs in gcc is unnecessary :)
struct node{
	int weight = 0;
};

std::atomic<node*> head;
//node* head;

void printWorld()
{
	
	int output = 100;
	
	for (int i = 0; i < 100000000; ++i)
	{
		__transaction_atomic
		{
			head->weight++;
		}
	}
	
	__transaction_atomic
	{
		output = head->weight;
	}
	
	__transaction_relaxed
	{
		std::cout << "Output is " << output << std::endl;
	}
}

int main()
{
	head = new node();
	head->weight = 0;
	std::thread t1(printWorld);
	std::thread t2(printWorld);
	std::thread t3(printWorld);

	t1.join();
	t2.join();
	t3.join();
	return 0;
}
