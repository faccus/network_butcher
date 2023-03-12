//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_GRAPH_TRAITS_H
#define NETWORK_BUTCHER_GRAPH_TRAITS_H

#include "Graph.h"
#include "MWGraph.h"
#include "WGraph.h"

// using layer_type = Graph<node_type>;
using graph_type = network_butcher_types::MWGraph<graph_input_type>;
using block_graph_type = network_butcher_types::WGraph<std::pair<std::size_t, std::shared_ptr<node_id_collection_type>>>;

#endif // NETWORK_BUTCHER_GRAPH_TRAITS_H
