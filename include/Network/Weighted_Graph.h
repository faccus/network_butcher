//
// Created by faccus on 2/19/23.
//

#ifndef NETWORK_BUTCHER_WEIGHTED_GRAPH_H
#define NETWORK_BUTCHER_WEIGHTED_GRAPH_H

#include "Basic_traits.h"


template<class Base_Weighted_Graph>
class Weighted_Graph {
public:
  using Node_Id_Type = std::size_t;
  using Edge_Type = std::pair<Node_Id_Type, Node_Id_Type>;



  weight_type
  get_weight(Edge_Type const &edge) const;

  std::size_t
  size() const;



  std::set<Node_Id_Type>
  get_input_nodes(Node_Id_Type const& id) const;

  std::set<Node_Id_Type>
  get_output_nodes(Node_Id_Type const& id) const;

  std::pair<std::set<Node_Id_Type>, std::set<Node_Id_Type>>
  get_neighbors(Node_Id_Type const& id) const;




  Weighted_Graph(Base_Weighted_Graph &g) : graph(g) {}

private:
  Base_Weighted_Graph &graph;
};

#endif // NETWORK_BUTCHER_WEIGHTED_GRAPH_H