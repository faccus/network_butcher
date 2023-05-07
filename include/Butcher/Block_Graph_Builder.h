//
// Created by faccus on 24/04/23.
//

#ifndef NETWORK_BUTCHER_BLOCK_GRAPH_BUILDER_H
#define NETWORK_BUTCHER_BLOCK_GRAPH_BUILDER_H

#include "Extra_Constraint.h"
#include "Graph_traits.h"

namespace network_butcher
{
  /// A block graph Builder. Used by butcher to construct the graph onto which the K-shortest path algorithm is then
  /// applied \tparam GraphType The type of the input graph
  template <typename GraphType>
  class Block_Graph_Builder
  {
  protected:
    GraphType const &original_graph;

  public:
    explicit Block_Graph_Builder(GraphType const &original_graph)
      : original_graph(original_graph){};

    /// The basic construct method. It will produce the block graph using the specified options.
    /// \return The resulting block graph
    virtual block_graph_type
    construct_block_graph() = 0;

    virtual ~Block_Graph_Builder() = default;
  };
} // namespace network_butcher

#endif // NETWORK_BUTCHER_BLOCK_GRAPH_BUILDER_H