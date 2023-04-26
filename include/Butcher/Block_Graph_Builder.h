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
    GraphType const                     &original_graph;
    block_graph_type                     new_graph;
    std::map<node_id_type, node_id_type> old_to_new;

  public:
    explicit Block_Graph_Builder(GraphType const &original_graph)
      : original_graph{original_graph} {};

    virtual void
    construct_block_graph() = 0;

    block_graph_type &&
    get_block_graph();

    std::map<node_id_type, node_id_type> &&
    get_old_to_new();

    virtual ~Block_Graph_Builder() = default;
  };

  template <typename GraphType>
  block_graph_type &&
  Block_Graph_Builder<GraphType>::get_block_graph()
  {
    return std::move(this->new_graph);
  }

  template <typename GraphType>
  std::map<node_id_type, node_id_type> &&
  Block_Graph_Builder<GraphType>::get_old_to_new()
  {
    return std::move(this->old_to_new);
  }
} // namespace network_butcher

#endif // NETWORK_BUTCHER_BLOCK_GRAPH_BUILDER_H
