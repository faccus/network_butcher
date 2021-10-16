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

  std::transform(std::execution::seq,
                 graph.nodes.cbegin(),
                 graph.nodes.cend(),
                 memory_usages.begin(),
                 [&graph, include_parameters](node_type const &node) {
                   memory_type res = 0;

                   for (auto const &in : node.get_input())
                     {
                       auto const p = graph.nodes_content.find(in);
                       if (p != graph.nodes_content.cend())
                         res += p->second->compute_memory_usage();
                     }

                   for (auto const &out : node.get_output())
                     {
                       auto const p = graph.nodes_content.find(out);
                       if (p != graph.nodes_content.cend())
                         res += p->second->compute_memory_usage();
                     }

                   if (include_parameters)
                     for (auto const &param : node.get_parameters())
                       {
                         auto const p = graph.nodes_content.find(param);
                         if (p != graph.nodes_content.cend())
                           res += p->second->compute_memory_usage();
                       }

                   return res;
                 });

  return memory_usages;
}

std::vector<memory_type>
Computer::compute_nodes_memory_usage_input(const Graph<graph_input_type> &graph,
                                           bool include_parameters) const
{
  std::vector<memory_type> memory_usages;
  memory_usages.resize(graph.nodes.size());

  std::transform(std::execution::seq,
                 graph.nodes.cbegin(),
                 graph.nodes.cend(),
                 memory_usages.begin(),
                 [&graph, include_parameters](node_type const &node) {
                   memory_type res = 0;

                   for (auto const &in : node.get_input())
                     {
                       auto const p = graph.nodes_content.find(in);
                       if (p != graph.nodes_content.cend())
                         res += p->second->compute_memory_usage();
                     }

                   if (include_parameters)
                     for (auto const &param : node.get_parameters())
                       {
                         auto const p = graph.nodes_content.find(param);
                         if (p != graph.nodes_content.cend())
                           res += p->second->compute_memory_usage();
                       }

                   return res;
                 });

  return memory_usages;
}