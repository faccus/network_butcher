//
// Created by root on 07/03/22.
//

#ifndef NETWORK_BUTCHER_HEAP_TRAITS_H
#define NETWORK_BUTCHER_HEAP_TRAITS_H

#include "basic_traits.h"
#include "heap_eppstein.h"
#include "path_info.h"
#include "weighted_graph.h"


namespace network_butcher::kfinder
{
  template <typename Weight_Type = weight_type>
  using t_H_out_collection = std::unordered_map<node_id_type, H_out<t_edge_info<Weight_Type>, std::greater<>>>;

  template <typename Weight_Type = weight_type>
  using H_out_pointer = t_H_out_collection<Weight_Type>::const_iterator;

  template <typename Weight_Type = weight_type>
  struct pointer_greater
  {
    std::less<> comp{};

    bool
    operator()(t_H_out_collection<Weight_Type>::const_iterator const &lhs,
               t_H_out_collection<Weight_Type>::const_iterator const &rhs) const
    {
      return comp(rhs->second, lhs->second);
    }
  };

  template <typename Weight_Type = weight_type>
  using t_H_g_collection =
    std::unordered_map<node_id_type, Heap<H_out_pointer<Weight_Type>, pointer_greater<Weight_Type>>>;
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_TRAITS_H
