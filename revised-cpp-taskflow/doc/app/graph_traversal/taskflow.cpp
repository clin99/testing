#include <iostream>
#include <chrono>

#include <taskflow/taskflow.hpp>
#include "levelgraph.hpp"

struct TF {
  
  TF(LevelGraph& graph) {

    tasks.resize(graph.level()); 
    for(size_t i=0; i<tasks.size(); ++i) {
      tasks[i].resize(graph.length());
    }
  
    for(size_t i=0; i<graph.length(); i++){
      Node& n = graph.node_at(graph.level()-1, i); 
      tasks[graph.level()-1][i] = tf.silent_emplace([&](){ n.mark(); });
    }
  
    for(int l=graph.level()-2; l>=0 ; l--){
      for(size_t i=0; i<graph.length(); i++){
        Node& n = graph.node_at(l, i);
        tasks[l][i] = tf.silent_emplace([&](){ n.mark();});
        for(size_t k=0; k<n._out_edges.size(); k++){
          tasks[l][i].precede(tasks[l+1][n._out_edges[k]]);
        } 
      }
    }
  }

  void run() {
    tf.wait_for_all(); 
  }

  tf::Taskflow tf {std::thread::hardware_concurrency()};
  std::vector<std::vector<tf::Task>> tasks;

};

void traverse_regular_graph_taskflow(LevelGraph& graph){
  TF tf(graph);
  tf.run();
}

std::chrono::microseconds measure_time_taskflow(LevelGraph& graph){
  auto beg = std::chrono::high_resolution_clock::now();
  traverse_regular_graph_taskflow(graph);
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - beg);
}

//int main(int argc, char* argv[]){
//
//  for(int i=1; i<=200; i++){
//
//    LevelGraph graph(i, i);
//    auto taskflow = measure_time_taskflow(graph);
//    std::cout << "Level graph:\t" << i << "\tby\t" << i << std::endl;
//    std::cout << "Elasped time taskflow:\t" << taskflow << std::endl;
//    std::cout << "Graph is fully traversed:\t" << graph.validate_result() << std::endl;  
//    graph.clear_graph();
//    std::cout << std::endl;
//  }
//
//}
