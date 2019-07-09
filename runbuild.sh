mkdir -p build
cd build 
cmake ..
make
./unit_tests.x
./performance_benchmarks 500000
cd ..