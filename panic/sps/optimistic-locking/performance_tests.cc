#include <algorithm>
#include <iostream>
#include <limits>
#include <stack>
#include <queue>
#include <stdio.h>
#include <thread>
#include <random>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include "PaVTBST.h"

std::vector<int> init_list_ints(int num) {
  std::vector<int> vector;
  for (int i=0; i<num; i++) {
    vector.push_back(i);
  }
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(vector), std::end(vector), rng);
  return vector;
}

std::vector<Node *> init_list(int num) {
  std::vector<Node *> vector;
  for (int i=0; i<num; i++) {
    Node *n = new Node(i);
    vector.push_back(n);
  }
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(vector), std::end(vector), rng);
  return vector;
}

void routine_4(PaVTBST &bst, 
              int id, int n_threads, 
              std::vector<int> keys, 
              std::vector<int> ops) {

  int count = 0;
  int add = id;
  int rem = id;
  int cont = id;
  for (int i=0; i<ops.size(); i++) {
    if (ops.at(i)==0) {
      bst.insert(keys.at(add));
      add+=n_threads;
    } else if (ops.at(i) == 1) {
      bst.remove(keys.at(rem));
      rem+=n_threads;
    } else {
      bst.contains(keys.at(cont));
      rem+=n_threads;
    }
  }
}

std::vector<int> init_ops(int max, int add, int rem, int cont) {
  std::vector<int> ops;
  for (int i=0; i<add; i++) {
    ops.push_back(0);
  }
  for (int i=0; i<rem; i++) {
    ops.push_back(1);
  }
  for (int i=0; i<cont; i++) {
    ops.push_back(2);
  }
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(ops), std::end(ops), rng);
  return ops;
}

PaVTBST *init_BST(int numberOfNodes, bool AVL, std::random_device &rd) {

  PaVTBST *bst = new PaVTBST(AVL);

  int min = -1*numberOfNodes;
  std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
  std::uniform_int_distribution<int> uni(min,0); // guaranteed unbiased
  auto random_integer = uni(rng);

  int start = random_integer;
  int end = 2*numberOfNodes + start;

  std::vector<int> nodevals;
  int count = 0;
  for (int i=start; i<end; i+=2) {
    nodevals.push_back(i);
    count++;
  }
  std::shuffle(std::begin(nodevals), std::end(nodevals), rng);

  for (int i=0; i<nodevals.size(); i++) {
    bst->insert(nodevals.at(i));
  }

  return bst;
}


int main(int argc, char **argv) {
  
  if (argc!=6) {
    std::cout<<"Please enter correct number of arguments!"<<std::endl;
    return -1;
  }

  // Initialize device
  std::random_device rd;
  const std::vector<int> numThreads= {1, 2, 4, 8};
  for (int t = 0; t<numThreads.size(); t++ ) {

    int n_threads = numThreads.at(t);
    std::thread threads[n_threads];

    bool avlProp = std::atoi(argv[1]);

    int total = std::atoi(argv[2])/n_threads;
    int add = std::atoi(argv[3]);
    int rem = std::atoi(argv[4]);
    int cont = std::atoi(argv[5]);

    std::vector<int> keys = init_list_ints(total*n_threads);
    std::vector<int> ops = init_ops(total, total*add/100, total*rem/100, total*cont/100);

    double avg = 0;
    int runs = 1;
    for (int run=0; run<runs; run++) {
      PaVTBST *bst = init_BST(total*n_threads*add/100, avlProp, rd);
      std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      for (int i=0; i<n_threads; i++) {
      	threads[i] = std::thread(routine_4, std::ref(*bst), i, n_threads, std::ref(keys), std::ref(ops));  
      }
      for (int i=0; i<n_threads; i++) {
        threads[i].join();
      }
      std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> time_span = t2 - t1;

      avg += time_span.count();
      delete bst;
    }
    std::cout<<avg/((double) runs)<<std::endl;
  }
}