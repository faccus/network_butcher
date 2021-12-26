//
// Created by faccus on 30/10/21.
//

#ifndef NETWORK_BUTCHER_HEAP_EPPSTEIN_H
#define NETWORK_BUTCHER_HEAP_EPPSTEIN_H

#include "../Traits/Node_traits.h"
#include "Heap.h"
#include <limits>
#include <memory>
#include <utility>

struct edge_info
{
  std::shared_ptr<edge_type> edge;
  type_weight                delta_weight;

  edge_info(edge_type const &in_edge, type_weight const &in_delta_weight)
    : edge(std::make_shared<edge_type>(in_edge))
    , delta_weight(in_delta_weight)
  {}

  edge_info(std::shared_ptr<edge_type> in_edge,
            type_weight const         &in_delta_weight)
    : edge(std::move(in_edge))
    , delta_weight(in_delta_weight)
  {}

  constexpr bool
  operator<(const edge_info &rhs) const
  {
    return delta_weight < rhs.delta_weight ||
           (delta_weight == rhs.delta_weight && *edge < *rhs.edge);
  }

  constexpr bool
  operator>(const edge_info &rhs) const
  {
    return delta_weight > rhs.delta_weight ||
           (delta_weight == rhs.delta_weight && *edge > *rhs.edge);
  }
};

template <class T>
struct H_out
{
  Heap<T> heap;

  bool
  operator<(const H_out &rhs) const
  {
    if (!heap.children.empty() && !rhs.heap.children.empty())
      return *heap.children.cbegin() < *rhs.heap.children.cbegin();
    else if (heap.children.empty() && rhs.heap.children.empty())
      return true;
    else
      return heap.children.empty();
  }
};

using H_out_pointer = std::shared_ptr<H_out<edge_info>>;

bool
operator<(H_out_pointer const &lhs, H_out_pointer const &rhs);

using H_g         = Heap<H_out_pointer>;
using H_g_pointer = std::shared_ptr<H_g>;


#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
