//
// Created by faccus on 2/19/23.
//

#ifndef NETWORK_BUTCHER_WEIGHTED_GRAPH_H
#define NETWORK_BUTCHER_WEIGHTED_GRAPH_H

#include "Basic_traits.h"

namespace network_butcher_kfinder
{
  template<class Base_Weighted_Graph, class Base_Node_Type=Base_Weighted_Graph::Node_Type>
  class Weighted_Graph {

  public:
    using Node_Id_Type = std::size_t;
    using Edge_Type = std::pair<Node_Id_Type, Node_Id_Type>;
    using Graph_Type = Base_Weighted_Graph;
    using Node_Type = Base_Node_Type;



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

    std::vector<std::pair<std::set<Node_Id_Type>, std::set<Node_Id_Type>>> const &
    get_neighbors() const;




    Node_Type const &
    operator[](Node_Id_Type const &id) const;

    std::vector<Node_Type> const &
    get_nodes() const;




    explicit Weighted_Graph(Graph_Type const &g) : graph(g) {}

    Weighted_Graph(Weighted_Graph const &) = delete;
    Weighted_Graph operator=(Weighted_Graph const &) = delete;
    Weighted_Graph(Weighted_Graph &&) = delete;
    Weighted_Graph operator=(Weighted_Graph &&) = delete;
  private:
    Graph_Type const &graph;
  };
}

#endif // NETWORK_BUTCHER_WEIGHTED_GRAPH_H