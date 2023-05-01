//
// Created by faccus on 24/04/23.
//

#ifndef NETWORK_BUTCHER_BLOCK_GRAPH_BUILDER_H
#define NETWORK_BUTCHER_BLOCK_GRAPH_BUILDER_H

#include "Extra_Constraint.h"
#include "Graph_traits.h"

namespace network_butcher
{
  template <typename GraphType>
  class Block_Graph_Builder
  {
  protected:
    GraphType const &original_graph;

  public:
    explicit Block_Graph_Builder(GraphType const &original_graph)
      : original_graph(original_graph){};

    virtual block_graph_type
    construct_block_graph() = 0;

    virtual ~Block_Graph_Builder() = default;
  };
} // namespace network_butcher

#endif // NETWORK_BUTCHER_BLOCK_GRAPH_BUILDER_H