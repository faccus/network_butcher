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
    unsigned long long int> : Base_Weighted_Graph
  {
  public:
    using Weight_Type = unsigned long long int;

    using Edge_Type = std::pair<Node_Id_Type, Node_Id_Type>;

    using Graph_Type =
      network_butcher::types::WGraph<Parallel_Edges, network_butcher::types::Node, unsigned long long int>;
    using Weight_Edge_Type = std::multiset<Weight_Type>;

    using Node_Type            = typename Graph_Type::Node_Type;
    using Node_Collection_Type = std::vector<Node_Type>;


    [[nodiscard]] auto
    get_weight(Edge_Type const &edge) const -> Weight_Edge_Type
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

    [[nodiscard]] auto
    size() const -> std::size_t
    {
      return graph.size();
    };

    [[nodiscard]] auto
    empty() const -> bool
    {
      return graph.empty();
    };

    [[nodiscard]] auto
    get_output_nodes(Node_Id_Type const &id) const -> std::set<Node_Id_Type> const &
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


    auto
    operator[](Node_Id_Type const &id) const -> Node_Type const &
    {
      return graph[id];
    };

    [[nodiscard]] auto
    cbegin() const
    {
      return graph.cbegin();
    }

    [[nodiscard]] auto
    cend() const
    {
      return graph.cend();
    }

    [[nodiscard]] auto
    begin() const
    {
      return cbegin();
    }

    [[nodiscard]] auto
    end() const
    {
      return cend();
    }


    auto
    reverse() const -> Weighted_Graph<Graph_Type, !Reversed, Node_Type, Node_Collection_Type, Weight_Type>
    {
      return Weighted_Graph<Graph_Type, !Reversed, Node_Type, Node_Collection_Type, Weight_Type>(graph);
    }


    explicit Weighted_Graph(Graph_Type const &g)
      : Base_Weighted_Graph()
      , graph(g)
    {}

    ~Weighted_Graph() override = default;

  private:
    Graph_Type const &graph;
  };
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_WEIGHTED_GRAPH_SPECIALIZATION_H
