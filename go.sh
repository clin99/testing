set -x
g++ -ggdb3 -O2 -std=c++1z ./test.cpp -I . -I ./cpp-taskflow/ -lpthread
