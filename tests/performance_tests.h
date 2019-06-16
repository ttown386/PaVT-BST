#ifndef PAVT_PERFORMANCE_TESTS_H_
#define PAVT_PERFORMANCE_TESTS_H_
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <thread>
#include <random>
#include <sstream>
#include <unordered_map>
#include <chrono>

#include <PaVT/bst.h>

using namespace pavt;

typedef BST* (*data_structure_function)(const int&, std::random_device&);

struct BenchmarkData {
  int total_operations;
  int insert_count;
  int remove_count;
  int contains_count;
  int n_threads;
  BenchmarkData();
  BenchmarkData(int ops, int insert, int remove, int contains, int threads): 
    total_operations(ops),
    insert_count(insert),
    remove_count(remove),
    contains_count(contains),
    n_threads(threads)
    {}
};

void do_benchmarking(int total_ops);
void benchmark_data_structure(const int total_ops, const int struct_i,
                              std::random_device& rd);
void setup_and_run_benchmark_on_data_structure(const int total_ops,
                                               const int struct_i,
                                               const int benchmark_i,
                                               std::random_device& rd);
BenchmarkData setup_benchmark_data(const int n_threads,
                                   const int total_ops,
                                   const int percent_insert,
                                   const int percent_remove,
                                   const int percent_contains);
std::vector<double> run_benchmark(std::random_device& rd,
                                  BenchmarkData& benchmark_data,
                                  const int struct_i);
BST* init_BST(int numberOfNodes, bool AVL, std::random_device& rd);
BST* load_BST(const int& number_of_nodes, std::random_device& rd);
BST* load_AVL(const int& number_of_nodes, std::random_device& rd);
BST* initialize_data_structure(const int number_of_nodes, std::random_device& rd);
double measure_run(BST* bst, const int n_threads,
                   const std::vector<int> input_keys,
                   const std::vector<int> op_list);
void launch_threads(BST* bst, const int n_threads,
                    const std::vector<int> input_keys,
                    const std::vector<int> op_list);
std::vector<int> init_random_list_of_ints(int num);
std::vector<Node* > init_list(int num);
std::vector<int> init_ops(int add, int rem, int cont);
void routine_4(BST &bst, int id, int n_threads, 
               std::vector<int> keys, std::vector<int> ops);
void print_header(const std::string& datastructure_name);
void print_header();
void print_footer();
void print_intermediate();
void print_data_structure_results(
    const std::unordered_map<int, std::vector<double>>&  thread_run,
    const int benchmark_i);
void print_results(const std::vector<double>& run_times, 
                  const std::string& benchmark, const std::string& thread);
std::string string_precision(double val, int precision);
void output_params(const std::string& name, const std::string& threads,
                   const std::string& mean, const std::string& std,
                   const std::string& min, const std::string& max);
double average(const std::vector<double>& vec);
double standard_deviation(const std::vector<double>& vec);
double max(const std::vector<double>& vec);
double min(const std::vector<double>& vec);
#endif // PAVT_PERFORMANCE_TESTS_H_

