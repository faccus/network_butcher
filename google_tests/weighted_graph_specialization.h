//
// Created by faccus on 23/05/23.
//

#ifndef NETWORK_BUTCHER_WEIGHTED_GRAPH_SPECIALIZATION_H
#define NETWORK_BUTCHER_WEIGHTED_GRAPH_SPECIALIZATION_H

#include "weighted_graph.h"
#include "wgraph.h"

namespace network_butcher::kfinder
{
  template <bool Parallel_Edges, bool Reversed>
  class Weighted_Graph<
    network_butcher::types::WGraph<Parallel_Edges, network_butcher::types::Node, unsigned long long int>,
    Reversed,
    network_butcher::types::Node,
    std::vector<network_butcher::types::Node>,
    unsigned long long int> : base_Weighted_Graph
  {
  public:
    using Weight_Type = unsigned long long int;

    using Edge_Type = std::pair<node_id_type, node_id_type>;

    using Graph_Type =
      network_butcher::types::WGraph<Parallel_Edges, network_butcher::types::Node, unsigned long long int>;
    using Weight_Edge_Type = std::multiset<Weight_Type>;

    using Node_Type            = typename Graph_Type::Node_Type;
    using Node_Collection_Type = std::vector<Node_Type>;


    [[nodiscard]] Weight_Edge_Type
    get_weight(Edge_Type const &edge) const
    {
      if constexpr (Reversed && Parallel_Edges)
        {
          return graph.get_weight(std::make_pair(edge.second, edge.first));
        }
      else if constexpr (Reversed)
        {
          return {graph.get_weight(std::make_pair(edge.second, edge.first))};
        }
      else if constexpr (Parallel_Edges)
        {
          return graph.get_weight(edge);
        }
      else
        {
          return {graph.get_weight(edge)};
        }
    }

    [[nodiscard]] std::size_t
    size() const
    {
      return graph.size();
    };

    [[nodiscard]] bool
    empty() const
    {
      return graph.empty();
    };

    [[nodiscard]] std::set<node_id_type> const &
    get_output_nodes(node_id_type const &id) const
    {
      if constexpr (Reversed)
        {
          return graph.get_input_nodes(id);
        }
      else
        {
          return graph.get_output_nodes(id);
        }
    };


    Node_Type const &
    operator[](node_id_type const &id) const
    {
      return graph[id];
    };

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    cbegin() const
    {
      return graph.cbegin();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    cend() const
    {
      return graph.cend();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    begin() const
    {
      return cbegin();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    end() const
    {
      return cend();
    }


    Weighted_Graph<Graph_Type, !Reversed, Node_Type, Node_Collection_Type, Weight_Type>
    reverse() const
    {
      return Weighted_Graph<Graph_Type, !Reversed, Node_Type, Node_Collection_Type, Weight_Type>(graph);
    }


    explicit Weighted_Graph(Graph_Type const &g)
      : base_Weighted_Graph()
      , graph(g)
    {}

    ~Weighted_Graph() override = default;

  private:
    Graph_Type const &graph;
  };
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_WEIGHTED_GRAPH_SPECIALIZATION_H
