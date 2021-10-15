//
// Created by faccus on 15/10/21.
//
#include "Computer.h"

std::vector<memory_type>
Computer::compute_nodes_memory_usage(const Graph<graph_input_type> &graph,
                                     bool include_parameters) const
{
  std::vector<memory_type> memory_usages;
  memory_usages.resize(graph.nodes.size());

  std::transform(std::execution::par,
                 graph.nodes.cbegin(),
                 graph.nodes.cend(),
                 memory_usages.begin(),
                 [&graph, include_parameters](node_type const &node)
                 {
                   memory_type res = 0;

                   for (auto const &in : node.get_input())
                     res += graph.nodes_content[in]->compute_memory_usage();
                   for (auto const &out : node.get_output())
                     res += graph.nodes_content[out]->compute_memory_usage();

                   if (include_parameters)
                     for (auto const &param : node.get_parameters())
                       res +=
                         graph.nodes_content[param]->compute_memory_usage();
                   return res;
                 }
                 );

  return memory_usages;
}

std::vector<memory_type>
Computer::compute_nodes_memory_usage_input(const Graph<graph_input_type> &graph,
                                           bool include_parameters) const
{
  std::vector<memory_type> memory_usages;
  memory_usages.resize(graph.nodes.size());

  std::transform(std::execution::par,
                 graph.nodes.cbegin(),
                 graph.nodes.cend(),
                 memory_usages.begin(),
                 [&graph, include_parameters](node_type const &node)
                 {
                   memory_type res = 0;

                   for (auto const &in : node.get_input())
                     res += graph.nodes_content[in]->compute_memory_usage();

                   if (include_parameters)
                     for (auto const &param : node.get_parameters())
                       res +=
                         graph.nodes_content[param]->compute_memory_usage();
                   return res;
                 }
  );

  return memory_usages;
}