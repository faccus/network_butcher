//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_GRAPH_TRAITS_H
#define NETWORK_BUTCHER_GRAPH_TRAITS_H

#include "../../Network/Graph.h"

// using layer_type = Graph<node_type>;
using graph_type = Graph<graph_input_type>;
using slice_type = std::set<node_id_type>;

#endif // NETWORK_BUTCHER_GRAPH_TRAITS_H
