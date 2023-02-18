//
// Created by faccus on 30/10/21.
//

#ifndef NETWORK_BUTCHER_HEAP_EPPSTEIN_H
#define NETWORK_BUTCHER_HEAP_EPPSTEIN_H

#include "Node_traits.h"
#include "Heap.h"
#include <limits>
#include <memory>
#include <utility>

namespace network_butcher_kfinder
{
  struct edge_info
  {
    std::shared_ptr<edge_type> edge;
    weight_type                delta_weight;

    edge_info(edge_type const &in_edge, weight_type const &in_delta_weight)
      : edge(std::make_shared<edge_type>(in_edge))
      , delta_weight(in_delta_weight)
    {}

    edge_info(std::shared_ptr<edge_type> in_edge, weight_type const &in_delta_weight)
      : edge(std::move(in_edge))
      , delta_weight(in_delta_weight)
    {}

    constexpr bool
    operator<(const edge_info &rhs) const
    {
      return delta_weight < rhs.delta_weight || (delta_weight == rhs.delta_weight && *edge < *rhs.edge);
    }

    constexpr bool
    operator>(const edge_info &rhs) const
    {
      return delta_weight > rhs.delta_weight || (delta_weight == rhs.delta_weight && *edge > *rhs.edge);
    }
  };

  template <class T = edge_info>
  class H_out
  {
  public:
    using container_type = typename Heap<T>::container_type;

    Heap<T> heap;

    bool
    operator<(const H_out &rhs) const
    {
      if (heap.children.empty())
        return true;
      if (rhs.heap.children.empty())
        return false;
      return *heap.children.cbegin() < *rhs.heap.children.cbegin();
    }
  };


  bool
  operator<(std::shared_ptr<H_out<edge_info>> const &lhs, std::shared_ptr<H_out<edge_info>> const &rhs);
} // namespace network_butcher_kfinder
#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
