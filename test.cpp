#include <iostream>
#include <atomic>

#include <chrono>

#include "thread_pool.hpp"

#include <taskflow/taskflow.hpp>

#include "taskflow/threadpool/threadpool.hpp"

#include "./speculative_threadpool.hpp"

int control = 1023;

struct Test1 {
  int a;
  void inc_a(int v) {
    a+=v;
    std::cout << "Test1.inc_a:" << a << std::endl;
  }
};

std::atomic<int> total;

void mywork(int a) {
  total += a;
}


int main() {
  int JOB_COUNT = 8000000;

  Test1 t1;
  t1.a = 0;

  auto tbeg = std::chrono::high_resolution_clock::now();

  if(false) {
  //if(true) {
    std::cout << "Profiling Jose\n";
    total = 0;

    Thread_pool pool(4);

    pool.add(&Test1::inc_a, t1, 3);

    for(int i = 0; i < JOB_COUNT; ++i) {
      pool.add(mywork, 1);
    }

    pool.wait_all();
  }
  else {
    std::cout << "Profiling Taskflow\n";
    total = 0;

    
    speculative_threadpool::BasicSpeculativeThreadpool<std::function> pool(4);
    pool.silent_async([&](){ t1.inc_a(3); });
    std::deque<std::function<void()>> deq;
    for(int i = 0; i < JOB_COUNT; ++i) {
      deq.emplace_back([&](){ mywork(1); });
    }
    pool.silent_async(std::move(deq));
    //for(int i = 0; i < JOB_COUNT; ++i) {
    //  pool.silent_async([&](){ mywork(1); });
    //}

    /*
    tf::Taskflow tf;

    tf.silent_emplace([&](){ t1.inc_a(3); });

    //auto sz = JOB_COUNT/4;
    //for(int i=0; i<4; i++){
    //  tf.silent_emplace([&, total=sz](){ 
    //    for(size_t i=0; i<total; i++){
    //      mywork(1);
    //    }
    //  });
    //}

    for(int i = 0; i < JOB_COUNT; ++i) {
      tf.silent_emplace([&](){ mywork(1); });
    }

    tf.wait_for_all();
    auto tend = std::chrono::high_resolution_clock::now();
    std::cout << "No clear = " << std::chrono::duration<double>(tend-tbeg).count() << " s \n";
    */
  }

  auto tend = std::chrono::high_resolution_clock::now();
  std::cout << "Runtime = " << std::chrono::duration<double>(tend-tbeg).count() << " s \n";

  std::cout << "finished total:" << total << std::endl;
  std::cout << "test1.a:" << t1.a << std::endl;

  assert(t1.a == 3);
  assert(total == JOB_COUNT);

  //EXPECT_EQ(t1.a,3);
  //EXPECT_EQ(total,JOB_COUNT);
}

