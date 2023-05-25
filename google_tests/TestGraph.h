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
  using Dependencies_Type    = std::map<std::size_t, std::pair<std::set<std::size_t>, std::set<std::size_t>>>;

  Node_Collection_Type                                  nodes;
  Dependencies_Type                                     dependencies;
  std::map<std::pair<std::size_t, std::size_t>, double> map_weight;
};

template <typename T, bool t_Reversed>
class network_butcher::kfinder::Weighted_Graph<TestGraph<T>,
                                               t_Reversed,
                                               typename TestGraph<T>::Node_Type,
                                               typename TestGraph<T>::Node_Collection_Type,
                                               double> : base_Weighted_Graph
{
public:
  using Node_Id_Type = std::size_t;
  using Weight_Type  = weight_type;

  using Node_Type            = typename TestGraph<T>::Node_Type;
  using Node_Collection_Type = typename TestGraph<T>::Node_Collection_Type;

  using Edge_Type        = std::pair<Node_Id_Type, Node_Id_Type>;
  using Graph_Type       = TestGraph<T>;
  using Weight_Edge_Type = std::multiset<Weight_Type>;


  [[nodiscard]] Weight_Edge_Type
  get_weight(Edge_Type const &edge) const
  {
    if constexpr (t_Reversed)
      {
        return {graph.map_weight.find(std::make_pair(edge.second, edge.first))->second};
      }
    else
      {
        return {graph.map_weight.find(edge)->second};
      }
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
  get_output_nodes(Node_Id_Type const &id) const
  {
    if constexpr (t_Reversed)
      {
        return graph.dependencies.find(id)->second.first;
      }
    else
      {
        return graph.dependencies.find(id)->second.second;
      }
  };

  Node_Type const &
  operator[](Weight_Type const &id) const
  {
    return *graph.nodes.find(id);
  };


  typename Node_Collection_Type::const_iterator
  cbegin() const
  {
    return graph.nodes.cbegin();
  }

  typename Node_Collection_Type::const_iterator
  cend() const
  {
    return graph.nodes.cend();
  }

  typename Node_Collection_Type::const_iterator
  begin() const
  {
    return cbegin();
  }

  typename Node_Collection_Type::const_iterator
  end() const
  {
    return cend();
  }


  Weighted_Graph<Graph_Type, !t_Reversed, Node_Type, Node_Collection_Type, Weight_Type>
  reverse() const
  {
    return Weighted_Graph<Graph_Type, !t_Reversed, Node_Type, Node_Collection_Type, Weight_Type>(graph);
  }


  explicit Weighted_Graph(Graph_Type const &g)
    : base_Weighted_Graph()
    , graph(g)
  {}

  ~Weighted_Graph() override = default;

private:
  Graph_Type const &graph;
};


#endif // NETWORK_BUTCHER_TESTGRAPH_H
