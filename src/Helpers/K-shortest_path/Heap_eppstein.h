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
  edge_type   edge;
  type_weight delta_weight;
  constexpr bool
  operator<(const edge_info &rhs) const
  {
    return delta_weight < rhs.delta_weight ||
           (delta_weight == rhs.delta_weight && edge < rhs.edge);
  }

  constexpr bool
  operator>(const edge_info &rhs) const
  {
    return delta_weight > rhs.delta_weight ||
           (delta_weight == rhs.delta_weight && edge > rhs.edge);
  }
};

struct H_out
{
  Heap<edge_info> heap;

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

class H_g_content;
using H_g = Heap<H_g_content>;

class H_g_content
{
  std::shared_ptr<H_out> content_out;
  std::shared_ptr<H_g>   content_g;

public:

  H_g_content() = default;
  H_g_content(std::shared_ptr<H_out> p)
    : content_out(std::move(p))
  {}

  H_g_content(std::shared_ptr<H_g> p)
    : content_g(std::move(p))
  {}

  [[nodiscard]] edge_info
  get_value() const;
  bool
  operator<(H_g_content const &rhs) const;

  [[nodiscard]] std::set<edge_info>
  get_edges() const;
};

using H_g_pointer   = std::shared_ptr<H_g>;
using H_out_pointer = std::shared_ptr<H_out>;


#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
