//
// Created by faccus on 2/20/23.
//

#ifndef NETWORK_BUTCHER_TESTGRAPH_H
#define NETWORK_BUTCHER_TESTGRAPH_H

#include "Weighted_Graph.h"

template <class T>
class TestGraph
{
public:
  struct TestNode
  {
    std::size_t id;
    T           content;

    [[nodiscard]] std::size_t
    get_id() const
    {
      return id;
    }
  };

  using Node_Type            = TestNode;
  using Node_Collection_Type = std::vector<Node_Type>;

  Node_Collection_Type                                                           nodes;
  std::map<std::size_t, std::pair<std::set<std::size_t>, std::set<std::size_t>>> dependencies;
  std::map<std::pair<std::size_t, std::size_t>, double>                          map_weight;
};

template <class T>
class network_butcher::kfinder::Weighted_Graph<TestGraph<T>>
{
public:
  using Node_Id_Type         = std::size_t;
  using Edge_Type            = std::pair<Node_Id_Type, Node_Id_Type>;
  using Graph_Type           = TestGraph<T>;
  using Node_Type            = typename TestGraph<T>::Node_Type;
  using Node_Collection_Type = typename TestGraph<T>::Node_Collection_Type;


  [[nodiscard]] weight_type
  get_weight(Edge_Type const &edge) const
  {
    auto const it = graph.map_weight.find(edge);

    return it != graph.map_weight.cend() ? it->second : -1;
  };

  [[nodiscard]] std::size_t
  size() const
  {
    return graph.nodes.size();
  };

  [[nodiscard]] bool
  empty() const
  {
    return graph.nodes.empty();
  };


  [[nodiscard]] std::set<Node_Id_Type> const &
  get_input_nodes(Node_Id_Type const &id) const
  {
    return graph.dependencies.find(id)->second.first;
  };

  [[nodiscard]] std::set<Node_Id_Type> const &
  get_output_nodes(Node_Id_Type const &id) const
  {
    return graph.dependencies.find(id)->second.second;
  };


  Node_Type const &
  operator[](Node_Id_Type const &id) const
  {
    return *graph.nodes.find(id);
  };


  typename Node_Collection_Type::const_iterator
  cbegin() const
  {
    return graph.get_nodes().cbegin();
  }

  typename Node_Collection_Type::const_iterator
  cend() const
  {
    return graph.get_nodes().cend();
  }

  typename Node_Collection_Type::const_reverse_iterator
  crbegin() const
  {
    return graph.get_nodes().crbegin();
  }

  typename Node_Collection_Type::const_reverse_iterator
  crend() const
  {
    return graph.get_nodes().crend();
  }


  explicit Weighted_Graph(Graph_Type const &g)
    : graph(g)
  {}

  Weighted_Graph(Weighted_Graph const &) = delete;
  Weighted_Graph
  operator=(Weighted_Graph const &) = delete;
  Weighted_Graph(Weighted_Graph &&) = delete;
  Weighted_Graph
  operator=(Weighted_Graph &&) = delete;

private:
  Graph_Type const &graph;
};

#endif // NETWORK_BUTCHER_TESTGRAPH_H
