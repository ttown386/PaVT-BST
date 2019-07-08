#include <PaVT/avl.h>
#include <PaVT/Base/binary_tree.h>

#include "performance_tests.h"

const std::vector<int> numThreads = {1, 2, 4, 8, 16, 32};
const int insert_percent[] = {9, 20, 50};
const int remove_percent[] = {1, 10, 50};
const int contains_percent[] = {90, 70, 0};
const std::vector<std::string> benchmarks {
  "09-01-90",
  "20-10-70",
  "50-50-00"
};
const std::vector<std::string> datastructure {
  "BINARY SEARCH TREE",
  "AVL TREE"
};

const std::vector<data_structure_function> data_structure_map {
  load_BST,
  load_AVL
};

int main(int argc, char **argv) {
  
  if (argc!=2) {
    std::cout<<"Please enter the total number of operations !"<<std::endl;
    return -1;
  }
  int total_ops = std::atoi(argv[1]);
  do_benchmarking(total_ops);
}

void do_benchmarking(int total_ops) {
  for (std::size_t structure_i = 0; structure_i < datastructure.size(); structure_i++) {
    print_header(datastructure[structure_i]);
    std::random_device rd;
    benchmark_data_structure(total_ops, structure_i, rd); 
    print_footer();
  }
}


void benchmark_data_structure(const int total_ops, const int struct_i,
                              std::random_device& rd) {
  for (std::size_t benchmark_i=0; benchmark_i < benchmarks.size(); benchmark_i++) {
    setup_and_run_benchmark_on_data_structure(total_ops, struct_i, 
                                              benchmark_i, rd);
    print_intermediate();
  }
}


void setup_and_run_benchmark_on_data_structure(const int total_ops,
                                               const int struct_i,
                                               const int benchmark_i,
                                               std::random_device& rd) {
  std::unordered_map<int, std::vector<double>> thread_run;
  for (std::size_t t = 0; t<numThreads.size(); t++ ) {
    int n_threads = numThreads.at(t);

    BenchmarkData benchmark_data = setup_benchmark_data(
        n_threads, total_ops, insert_percent[benchmark_i],
        remove_percent[benchmark_i], contains_percent[benchmark_i]);

    std::vector<double> run = run_benchmark(rd, benchmark_data, struct_i);

    thread_run[numThreads.at(t)] = run;
  }
  print_data_structure_results(thread_run, benchmark_i);
}


BenchmarkData setup_benchmark_data(const int n_threads,
                                   const int total_ops,
                                   const int percent_insert,
                                   const int percent_remove,
                                   const int percent_contains) {
  int ops_per_thread = total_ops/n_threads;
  int num_insert_ops = ops_per_thread*percent_insert/100;
  int num_remove_ops = ops_per_thread*percent_remove/100;
  int num_contains_ops = ops_per_thread*percent_contains/100;

  BenchmarkData benchmark_data(total_ops, num_insert_ops, num_remove_ops,
                               num_contains_ops, n_threads);                                   
  return benchmark_data;
}


std::vector<double> run_benchmark(std::random_device& rd,
                                  BenchmarkData& benchmark_data,
                                  const int struct_i) {

  std::vector<double> run_data;

  int total_ops = benchmark_data.total_operations;
  int n_insert = benchmark_data.insert_count;
  int n_remove = benchmark_data.remove_count;
  int n_contains = benchmark_data.contains_count;
  int n_threads = benchmark_data.n_threads;

  std::vector<int> input_keys = init_random_list_of_ints(total_ops);
  std::vector<int> op_list = init_ops(n_insert, n_remove, n_contains);

  int runs = 10;
  for (int run=0; run<runs; run++) {
    BST *bst = data_structure_map[struct_i](n_insert, rd);
    double run_time = measure_run(bst, n_threads, input_keys, op_list);
    run_data.push_back(run_time); 
    delete bst;
  }
  return run_data;
}


BST* init_BST(int numberOfNodes, bool AVL, std::random_device& rd) {

  BST *bst = new BST();

  int min = -1*numberOfNodes;
  std::mt19937 rng(rd());    // random-number engine used (mersenne-twister in this case)
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

  for (std::size_t i=0; i<nodevals.size(); i++) {
    bst->insert(nodevals.at(i));
  }

  return bst;
}

BST* load_BST(const int& number_of_nodes, std::random_device& rd) {
  BST *bst = new BST();

  int min = -1*number_of_nodes;
  std::mt19937 rng(rd());    // random-number engine used (mersenne-twister in this case)
  std::uniform_int_distribution<int> uni(min,0); // guaranteed unbiased
  auto random_integer = uni(rng);

  int start = random_integer;
  int end = 2*number_of_nodes + start;

  std::vector<int> nodevals;
  int count = 0;
  for (int i=start; i<end; i+=2) {
    nodevals.push_back(i);
    count++;
  }
  std::shuffle(std::begin(nodevals), std::end(nodevals), rng);

  for (std::size_t i=0; i<nodevals.size(); i++) {
    bst->insert(nodevals.at(i));
  }

  return bst;
}

BST* load_AVL(const int& number_of_nodes, std::random_device& rd) {
  AVL *bst = new AVL();

  int min = -1*number_of_nodes;
  std::mt19937 rng(rd());    // random-number engine used (mersenne-twister in this case)
  std::uniform_int_distribution<int> uni(min,0); // guaranteed unbiased
  auto random_integer = uni(rng);

  int start = random_integer;
  int end = 2*number_of_nodes + start;

  std::vector<int> nodevals;
  int count = 0;
  for (int i=start; i<end; i+=2) {
    nodevals.push_back(i);
    count++;
  }
  std::shuffle(std::begin(nodevals), std::end(nodevals), rng);

  for (std::size_t i=0; i<nodevals.size(); i++) {
    bst->insert(nodevals.at(i));
  }

  return bst;
}


double measure_run(BST* bst, const int n_threads,
                   const std::vector<int> input_keys,
                   const std::vector<int> op_list) {
  auto t1 = std::chrono::high_resolution_clock::now();
  launch_threads(bst, n_threads, input_keys, op_list);
  auto t2 = std::chrono::high_resolution_clock::now();
  
  std::chrono::duration<double, std::milli> time_span = t2 - t1;
  return time_span.count();
}


void launch_threads(BST* bst, const int n_threads,
                    const std::vector<int> input_keys,
                    const std::vector<int> op_list) {
  std::thread threads[n_threads];
  for (int i=0; i<n_threads; i++) {
    threads[i] = std::thread(routine_4, std::ref(*bst), 
                             i, n_threads, std::ref(input_keys),
                             std::ref(op_list));  
  }
  for (int i=0; i<n_threads; i++) {
    threads[i].join();
  }
}


std::vector<int> init_random_list_of_ints(int num) {
  std::vector<int> vector;
  for (int i=0; i<num; i++) {
    vector.push_back(i);
  }
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(vector), std::end(vector), rng);
  return vector;
}


std::vector<BinaryTree::Node* > init_list(int num) {
  std::vector<BinaryTree::Node* > vector;
  for (int i=0; i<num; i++) {
    BinaryTree::Node* n = new BinaryTree::Node(i);
    vector.push_back(n);
  }
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(vector), std::end(vector), rng);
  return vector;
}


std::vector<int> init_ops(int add, int rem, int cont) {
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


void routine_4(BST &bst, int id, int n_threads, 
               std::vector<int> keys, std::vector<int> ops) {
  int add = id;
  int rem = id;
  int cont = id;
  for (std::size_t i=0; i<ops.size(); i++) {
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


void print_header(const std::string& datastructure_name) {
  std::cout << std::string(4, ' ') 
            << datastructure_name << std::endl;
  print_header();
}


void print_header() {
  const std::string top_tacks = std::string(23, '=');
  const std::string title = ":::: PERFORMANCE BENCHMARKS ::::";
  const std::string full_title = top_tacks + title + top_tacks;
  const std::string bottom = std::string (78, '=');
  std::cout << full_title << std::endl;
  output_params("BENCHMARK", "THREADS", "MEAN", "STD", " MIN", "MAX");
  std::cout << bottom << std::endl;
}


void print_footer() {
  std::cout << std::string(78, '=') << "\n\n";
}


void print_intermediate() {
  std::cout<<std::string (78, '.')<<std::endl;
}


void print_data_structure_results(
    const std::unordered_map<int, std::vector<double>>&  thread_run,
    const int benchmark_i) {
  for (auto thread_i = numThreads.begin(); thread_i != numThreads.end(); thread_i++) {
    print_results(thread_run.at(*thread_i), benchmarks[benchmark_i], std::to_string(*thread_i));
  }
}


void print_results(const std::vector<double>& run_times, 
                  const std::string& benchmark, const std::string& thread) {
  int precision = 2;
  double std_dev = standard_deviation(run_times);
  double mean = average(run_times);
  double maxval = max(run_times);
  double minval = min(run_times);
  std::string s_std_dev = string_precision(std_dev, precision);
  std::string s_average= string_precision(mean, precision);
  std::string s_min= string_precision(minval, precision);
  std::string s_max= string_precision(maxval, precision);
  output_params(benchmark, thread, s_average, s_std_dev, s_min, s_max);
}


std::string string_precision(double val, int precision) {
  std::stringstream stream;
  stream << std::fixed << std::setprecision(precision) << val;
  return stream.str();
}


void output_params(const std::string& name, const std::string& threads,
                   const std::string& mean, const std::string& std,
                   const std::string& min, const std::string& max) {
  std::cout << std::setw(15) << std::left << name << "   "
            << std::setw(10) << std::left << threads << "   "
            << std::setw(10) << std::right << mean << " +- "
            << std::setw(10) << std::left << std << "     "
            << std::setw(10) << std::left << min << "   "
            << std::setw(10) << std::left << max 
            << std::endl;
}


double sum(const std::vector<double>& vec) {
  double init_val = 0.0;
  return std::accumulate(std::begin(vec), std::end(vec), init_val);
}


double average(const std::vector<double>& vec) {
  return sum(vec)/vec.size();
}


double standard_deviation(const std::vector<double>& vec) {
  double mean = average(vec);
  std::vector<double> diff(vec.size());
  std::transform(vec.begin(), vec.end(), diff.begin(), [mean](double x) {
    return x - mean; 
  });
  double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  return std::sqrt(sq_sum / vec.size());
}


double max(const std::vector<double>& vec) {
  auto position_iterator = std::max_element(std::begin(vec), std::end(vec));
  return *position_iterator;
}


double min(const std::vector<double>& vec) {
  auto position_iterator = std::min_element(std::begin(vec), std::end(vec));
  return *position_iterator;
}