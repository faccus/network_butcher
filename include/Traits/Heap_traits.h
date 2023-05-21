//
// Created by root on 07/03/22.
//

#ifndef NETWORK_BUTCHER_HEAP_TRAITS_H
#define NETWORK_BUTCHER_HEAP_TRAITS_H

#include "Basic_traits.h"
#include "Heap_eppstein.h"
#include "Path_info.h"
#include "Weighted_Graph.h"


namespace network_butcher::kfinder
{
  using edge_pointer = edge_info;

  using full_H_out_type  = H_out<edge_pointer, std::greater<>>;
  using H_out_collection = std::unordered_map<node_id_type, full_H_out_type>;
  using H_out_pointer    = H_out_collection::const_iterator;

  struct pointer_greater
  {
    std::less<> comp{};

    bool
    operator()(H_out_collection::const_iterator const &lhs, H_out_collection::const_iterator const &rhs) const
    {
      return comp(rhs->second, lhs->second);
    }
  };

  using H_g            = Heap<H_out_pointer, pointer_greater>;
  using H_g_collection = std::unordered_map<node_id_type, H_g>;
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_TRAITS_H
