//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_BUTCHER_H
#define NETWORK_BUTCHER_BUTCHER_H

#include "Network/Graph.h"
#include "Network/Layer.h"
#include "Network/Node.h"
#include "Helpers/Types/Type_info.h"
#include "Helpers/Utilities.h"


using Type_info_pointer = std::shared_ptr<Type_info>;
using Node_type = Node<Type_info_pointer>;
using Layer_type        = Graph<Node_type>;
using In_graph_type = Node_type;

class Butcher
{
private:
  using Network = Graph<In_graph_type>;

  Network graph;

public:
  Butcher() = default;
  explicit Butcher(Network &&g)
    : graph(std::move(g)){};

  explicit Butcher(const std::string & p)
    : graph(p) {};

  std::vector<std::vector<int>>
  compute_two_slice_memory_brute_force(size_t memory_first_slice) const; // second = 0 -> cloud with infinite space

};


#endif // NETWORK_BUTCHER_BUTCHER_H
