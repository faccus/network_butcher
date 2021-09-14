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

  // WIP
  int
  recursive_brute_forcer(std::set<Slice_type> &,
                         const std::vector<Slice_type> &,
                         Slice_type &,
                         int,
                         int mode = -1,
                         int end  = 0);

  // Compute the minimal connected graphs containing every node
  std::vector<Slice_type>
  compute_basic_routes() const;

public:
  Butcher() = default;
  explicit Butcher(Network &&g)
    : graph(std::move(g)){};

  Butcher(const std::string &p, bool ignore_parameters = true, bool dep = true)
    : graph(p, ignore_parameters, dep){};


  std::vector<Slice_type>
  compute_two_slice_memory_brute_force(size_t memory_first_slice) const;


  std::vector<Slice_type>
  compute_partial_two_slice_memory_brute_force(size_t memory_first_slice) const;


  std::vector<Slice_type>
  compute_two_slice_brute_force() const;

  // WIP
  std::vector<Slice_type>
  partial_compute_two_slice_brute_force() const;
};


#endif // NETWORK_BUTCHER_BUTCHER_H
