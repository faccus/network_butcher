//
// Created by root on 07/03/22.
//

#ifndef NETWORK_BUTCHER_HEAP_TRAITS_H
#define NETWORK_BUTCHER_HEAP_TRAITS_H

#include <forward_list>
#include <list>

#include "../K-shortest_path/Heap.h"
#include "../K-shortest_path/Heap_eppstein.h"

using H_out_pointer = std::shared_ptr<H_out<edge_info>>;
using H_g         = Heap<H_out_pointer>;
using H_g_pointer = std::shared_ptr<H_g>;

using H_out_map = std::map<node_id_type, H_out_pointer>;
using H_g_map = std::map<node_id_type, H_g>;

using edge_sequence     = std::list<edge_pointer>;
using h_edge_edges_type = std::map<edge_pointer, edge_sequence>;
using edge_edges_type   = std::map<node_id_type, h_edge_edges_type>;

#endif // NETWORK_BUTCHER_HEAP_TRAITS_H
