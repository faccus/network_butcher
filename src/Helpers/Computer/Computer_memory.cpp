//
// Created by faccus on 15/10/21.
//
#include "Computer_memory.h"

std::vector<memory_type>
Computer_memory::compute_nodes_memory_usage(const Graph<graph_input_type> &graph,
                                     bool include_parameters)
{
  std::vector<memory_type> memory_usages;
  memory_usages.resize(graph.nodes.size());

  std::transform(graph.nodes.cbegin(),
                 graph.nodes.cend(),
                 memory_usages.begin(),
                 [&graph, include_parameters](node_type const &node) {
                   memory_type res = 0;

                   for (auto const &in : node.get_input())
                     {
                       auto const p = graph.nodes_content.find(in.second);
                       if (p != graph.nodes_content.cend())
                         res += p->second->compute_memory_usage();
                     }

                   for (auto const &out : node.get_output())
                     {
                       auto const p = graph.nodes_content.find(out.second);
                       if (p != graph.nodes_content.cend())
                         res += p->second->compute_memory_usage();
                     }

                   if (include_parameters)
                     for (auto const &param : node.get_parameters())
                       {
                         auto const p = graph.nodes_content.find(param.second);
                         if (p != graph.nodes_content.cend())
                           res += p->second->compute_memory_usage();
                       }

                   return res;
                 });

  return memory_usages;
}

std::vector<memory_type>
Computer_memory::compute_nodes_memory_usage_input(const Graph<graph_input_type> &graph,
                                           bool include_parameters)
{
  std::vector<memory_type> memory_usages;
  memory_usages.resize(graph.nodes.size());

  std::transform(graph.nodes.cbegin(),
                 graph.nodes.cend(),
                 memory_usages.begin(),
                 [&graph, include_parameters](node_type const &node) {
                   memory_type res = 0;

                   for (auto const &in : node.get_input())
                     {
                       auto const p = graph.nodes_content.find(in.second);
                       if (p != graph.nodes_content.cend())
                         res += p->second->compute_memory_usage();
                     }

                   if (include_parameters)
                     for (auto const &param : node.get_parameters())
                       {
                         auto const p = graph.nodes_content.find(param.second);
                         if (p != graph.nodes_content.cend())
                           res += p->second->compute_memory_usage();
                       }

                   return res;
                 });

  return memory_usages;
}