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
using Node_type = Node<Type_info>;

using Layer_type        = Graph<Node_type>;
using In_graph_type = Node_type;

using Slice_type = std::set<int>;

class Butcher
{
private:
  using Network = Graph<In_graph_type>;

  Network graph;


  int
  partial_two_slice_brute_force_helper(std::vector<Slice_type> &,
                                       int,
                                       int,
                                       int,
                                       std::vector<int>,
                                       std::vector<int>) const;

public:
  Butcher() = default;
  explicit Butcher(Network &&g)
    : graph(std::move(g)){};

  explicit Butcher(const std::string &p)
    : graph(p){};

  std::vector<Slice_type>
  compute_two_slice_memory_brute_force(
    size_t memory_first_slice) const; // second = 0 -> cloud with infinite space

  std::vector<Slice_type>
  compute_partial_two_slice_memory_brute_force(size_t memory_first_slice) const;

  std::vector<Slice_type>
  compute_two_slice_brute_force() const;
};


#endif // NETWORK_BUTCHER_BUTCHER_H
