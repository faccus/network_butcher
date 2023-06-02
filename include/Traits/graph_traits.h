//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_GRAPH_TRAITS_H
#define NETWORK_BUTCHER_GRAPH_TRAITS_H

#include "graph.h"
#include "mwgraph.h"
#include "wgraph.h"

#include "content.h"

namespace network_butcher
{
  // using layer_type = Graph<node_type>;
  using Converted_Onnx_Graph_Type = network_butcher::types::MWGraph<false, Onnx_Converted_Node_Type>;
  using Block_Graph_Type          = network_butcher::types::WGraph<false, Block_Graph_Node_Type>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_GRAPH_TRAITS_H
