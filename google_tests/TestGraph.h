//
// Created by faccus on 2/20/23.
//

#ifndef NETWORK_BUTCHER_TESTGRAPH_H
#define NETWORK_BUTCHER_TESTGRAPH_H

#include "Weighted_Graph.h"

template<class T>
class TestGraph {
public:
  std::vector<T> nodes;
  std::map<std::size_t, std::pair<std::set<std::size_t>, std::set<std::size_t>>> dependencies;
  std::map<std::pair<std::size_t, std::size_t>, double> map_weight;

  using Node_Type = T;
};

template<class T>
class network_butcher_kfinder::Weighted_Graph<TestGraph<T>> {
public:
  using Node_Id_Type = std::size_t;
  using Edge_Type = std::pair<Node_Id_Type, Node_Id_Type>;
  using Graph_Type = TestGraph<T>;
  using Node_Type = TestGraph<T>::Node_Type;



  weight_type
  get_weight(Edge_Type const &edge) const {
    return graph.map_weight[edge];
  };

  std::size_t
  size() const {
    return graph.nodes.size();
  };

  bool
  empty() const {
    return graph.nodes.empty();
  };



  std::set<Node_Id_Type> const &
  get_input_nodes(Node_Id_Type const &id) const {
    return graph.dependencies[id].first;
  };

  std::set<Node_Id_Type> const &
  get_output_nodes(Node_Id_Type const &id) const{
    return graph.dependencies[id].second;
  };

  std::vector<std::pair<std::set<Node_Id_Type>, std::set<Node_Id_Type>>> const &
  get_neighbors() const{
    return graph.dependencies;
  };




  Node_Type const &
  operator[](Node_Id_Type const &id) const {
    return graph.nodes[id];
  };

  std::vector<Node_Type> const &
  get_nodes() const {
    return graph.nodes;
  };




  explicit Weighted_Graph(Graph_Type const &g) : graph(g) {}

  Weighted_Graph(Weighted_Graph const &) = delete;
  Weighted_Graph operator=(Weighted_Graph const &) = delete;
  Weighted_Graph(Weighted_Graph &&) = delete;
  Weighted_Graph operator=(Weighted_Graph &&) = delete;
private:
  Graph_Type const &graph;
};

#endif // NETWORK_BUTCHER_TESTGRAPH_H
