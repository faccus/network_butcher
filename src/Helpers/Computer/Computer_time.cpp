//
// Created by faccus on 07/11/21.
//

#include "Computer_time.h"

void
Computer_time::setup() const
{
  auto &my_factory = get_factory();

  my_factory.add("Relu",
                 [](Graph<graph_input_type> const &graph,
                    Node const                    &node,
                    bool                           forward = true) {
                   std::size_t res = 0;
                   auto const  it =
                     graph.nodes_content.find(*node.get_output().begin());
                   if (it == graph.nodes_content.cend())
                     return res;

                   std::size_t const C_out = it->second->get_shape()[1];
                   res                     = forward ? 3 * C_out : 4 * C_out;

                   return res;
                 });

  my_factory.add("Loss",
                 [](Graph<graph_input_type> const &graph,
                    Node const                    &node,
                    bool                           forward = true) {
                   std::size_t res = 0;
                   auto const  it =
                     graph.nodes_content.find(*node.get_output().begin());
                   if (it == graph.nodes_content.cend())
                     return res;

                   std::size_t const C_out = it->second->get_shape()[1];
                   res = forward ? 4 * C_out - 1 : C_out + 1;

                   return res;
                 });

  my_factory.add("BatchNormalization",
                 [](Graph<graph_input_type> const &graph,
                    Node const                    &node,
                    bool                           forward = true) {
                   std::size_t res = 0;
                   auto const  it =
                     graph.nodes_content.find(*node.get_output().begin());
                   if (it == graph.nodes_content.cend())
                     return res;

                   auto const it2 =
                     graph.nodes_content.find(*node.get_input().begin());
                   if (it2 == graph.nodes_content.cend())
                     return res;

                   std::size_t const C_in  = it2->second->get_shape()[1];
                   std::size_t const C_out = it->second->get_shape()[1];
                   res = forward ? 5 * C_out + C_in - 2 : 8 * C_out + C_in - 1;

                   return res;
                 });
}

time_type
Computer_time::compute_time_usage(const Graph<graph_input_type> &graph,
                                  const Node                    &node)
{
  time_type res = .0;


  return res;
}


time_type
Computer_time::compute_time_usage(const Graph<graph_input_type> &graph,
                                  node_id_type                   id)
{
  return compute_time_usage(graph, graph.nodes[id]);
}
