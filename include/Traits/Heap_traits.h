//
// Created by root on 07/03/22.
//

#ifndef NETWORK_BUTCHER_HEAP_TRAITS_H
#define NETWORK_BUTCHER_HEAP_TRAITS_H

#include "Basic_traits.h"
#include "Weighted_Graph.h"
#include "Heap_eppstein.h"
#include "Path_info.h"

namespace network_butcher_kfinder
{
  using H_out_pointer = std::shared_ptr<H_out<edge_info>>;
  using H_g           = Heap<H_out_pointer>;

  using H_out_collection = std::unordered_map<node_id_type, H_out_pointer>;
  using H_g_collection   = std::unordered_map<node_id_type, H_g>;

  using edge_sequence     = std::vector<edge_pointer>;
  using h_edge_edges_type = std::map<edge_pointer, edge_sequence>;
  using edge_edges_type   = std::map<node_id_type, h_edge_edges_type>;
} // namespace network_butcher_kfinder

#endif // NETWORK_BUTCHER_HEAP_TRAITS_H
