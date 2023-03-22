//
// Created by faccus on 2/19/23.
//

#ifndef NETWORK_BUTCHER_WEIGHTED_GRAPH_H
#define NETWORK_BUTCHER_WEIGHTED_GRAPH_H

#include "Basic_traits.h"


namespace network_butcher
{
  namespace kfinder
  {
    template <class Base_Weighted_Graph,
              class Node_Type            = typename Base_Weighted_Graph::Node_Type,
              class Node_Collection_Type = typename Base_Weighted_Graph::Node_Collection_Type>
    class Weighted_Graph
    {
    public:
      using Node_Id_Type = std::size_t;
      using Edge_Type    = std::pair<Node_Id_Type, Node_Id_Type>;
      using Graph_Type   = Base_Weighted_Graph;


      weight_type
      get_weight(Edge_Type const &edge) const;

      std::size_t
      size() const;

      bool
      empty() const;


      std::set<Node_Id_Type> const &
      get_input_nodes(Node_Id_Type const &id) const;

      std::set<Node_Id_Type> const &
      get_output_nodes(Node_Id_Type const &id) const;


      Node_Type const &
      operator[](Node_Id_Type const &id) const;

      typename Node_Collection_Type::const_iterator
      cbegin() const;

      typename Node_Collection_Type::const_iterator
      cend() const;

      typename Node_Collection_Type::const_reverse_iterator
      crbegin() const;

      typename Node_Collection_Type::const_reverse_iterator
      crend() const;


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
  } // namespace kfinder

} // namespace network_butcher

#endif // NETWORK_BUTCHER_WEIGHTED_GRAPH_H