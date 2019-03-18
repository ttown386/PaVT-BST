mkdir -p build
cd build 
cmake ..
make VERBOSE=1 -j
cd ..