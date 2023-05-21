//
// Created by faccus on 2/19/23.
//

#ifndef NETWORK_BUTCHER_WEIGHTED_GRAPH_H
#define NETWORK_BUTCHER_WEIGHTED_GRAPH_H

#include "Basic_traits.h"


namespace network_butcher::kfinder
{
  /// Interface class for a Weighted Graph
  /// \tparam Base_Weighted_Graph The class of the weighted graph to represent
  /// \tparam Node_Type The type of a Node of the weight graph
  /// \tparam Node_Collection_Type The type of the collection containing the nodes of the graph
  template <typename Base_Weighted_Graph,
            typename Node_Type                  = typename Base_Weighted_Graph::Node_Type,
            typename Node_Collection_Type       = typename Base_Weighted_Graph::Node_Collection_Type,
            typename Dependencies_Type          = typename Base_Weighted_Graph::Dependencies_Type,
            bool Reversed                       = false,
            typename Reversed_Dependencies_Type = Dependencies_Type>
  class Weighted_Graph
  {
  public:
    using Node_Id_Type     = std::size_t;
    using Edge_Type        = std::pair<Node_Id_Type, Node_Id_Type>;
    using Graph_Type       = Base_Weighted_Graph;
    using Weight_Edge_Type = std::multiset<weight_type>;


    [[nodiscard]] Weight_Edge_Type
    get_weight(Edge_Type const &edge) const;

    [[nodiscard]] std::size_t
    size() const;

    [[nodiscard]] bool
    empty() const;


    [[nodiscard]] std::set<Node_Id_Type> const &
    get_output_nodes(Node_Id_Type const &id) const;


    Node_Type const &
    operator[](Node_Id_Type const &id) const;

    typename Node_Collection_Type::const_iterator
    cbegin() const;

    typename Node_Collection_Type::const_iterator
    cend() const;

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


    explicit Weighted_Graph(Graph_Type const &g)
      : graph(g)
    {}

    ~Weighted_Graph() = default;

  private:
    Graph_Type const                                              &graph;
    std::conditional_t<Reversed, Reversed_Dependencies_Type, bool> reversed_neighboors;
  };
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_WEIGHTED_GRAPH_H