//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_GRAPH_TRAITS_H
#define NETWORK_BUTCHER_GRAPH_TRAITS_H

#include "../Network/Graph.h"
#include "../Network/WGraph.h"
#include "../Network/MWGraph.h"

// using layer_type = Graph<node_type>;
using graph_type = network_butcher_types::MWGraph<graph_input_type>;

#endif // NETWORK_BUTCHER_GRAPH_TRAITS_H
