//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_COMPUTER_H
#define NETWORK_BUTCHER_COMPUTER_H

#include "Graph.h"
#include "Hardware_specifications.h"

class Computer
{
private:
  Hardware_specifications hw_spec;
public:

  template <class T>
  [[nodiscard]] inline memory_type
  compute_memory_usage(const T &in) const
  {
    return in.compute_memory_usage();
  }

  template <class T>
  [[nodiscard]] inline memory_type
  compute_memory_usage(const std::shared_ptr<T> &in) const
  {
    return in->compute_memory_usage();
  }

  template <class T>
  [[nodiscard]] inline memory_type
  compute_memory_usage(const std::unique_ptr<T> &in) const
  {
    return in->compute_memory_usage();
  }


  template <class T>
  memory_type
  compute_memory_usage(const Graph<T>  &graph,
                       const node_type &node,
                       bool             include_parameters) const
  {
    memory_type res = 0;

    for (auto const &in : node.get_input())
      {
        auto const p = graph.nodes_content.find(in);
        if (p != graph.nodes_content.cend())
          res += compute_memory_usage(p->second);
      }
    for (auto const &out : node.get_output())
      {
        auto const p = graph.nodes_content.find(out);
        if (p != graph.nodes_content.cend())
          res += compute_memory_usage(p->second);
      }

    if (include_parameters)
      for (auto const &param : node.get_parameters())
        {
          auto const p = graph.nodes_content.find(param);
          if (p != graph.nodes_content.cend())
            res += compute_memory_usage(p->second);
        }

    return res;
  }

  template <class T>
  memory_type
  compute_memory_usage_input(const Graph<T>  &graph,
                             const node_type &node,
                             bool             include_parameters) const
  {
    memory_type res = 0;

    for (auto const &in : node.get_input())
      {
        auto const p = graph.nodes_content.find(in);
        if (p != graph.nodes_content.cend())
          res += compute_memory_usage(p->second);
      }

    if (include_parameters)
      for (auto const &param : node.get_parameters())
        {
          auto const p = graph.nodes_content.find(param);
          if (p != graph.nodes_content.cend())
            res += compute_memory_usage(p->second);
        }

    return res;
  }



  template <class T>
  std::vector<memory_type>
  compute_nodes_memory_usage(Graph<T> const &graph,
                             bool            include_parameters = true) const
  {
    std::vector<memory_type> memory_usages;
    memory_usages.resize(graph.nodes.size());

    std::transform(graph.nodes.cbegin(),
                   graph.nodes.cend(),
                   memory_usages.begin(),
                   [&](node_type const &node) {
                     return compute_memory_usage(graph,
                                                 node,
                                                 include_parameters);
                   });

    return memory_usages;
  }

  [[nodiscard]] static std::vector<memory_type>
  compute_nodes_memory_usage(Graph<graph_input_type> const &graph,
                             bool include_parameters = true);


  template <class T>
  std::vector<memory_type>
  compute_nodes_memory_usage_input(Graph<T> const &graph,
                                   bool include_parameters = true) const
  {
    std::vector<memory_type> memory_usages;
    memory_usages.resize(graph.nodes.size());

    std::transform(graph.nodes.cbegin(),
                   graph.nodes.cend(),
                   memory_usages.begin(),
                   [&](node_type const &node) {
                     return compute_memory_usage_input(graph,
                                                       node,
                                                       include_parameters);
                   });

    return memory_usages;
  }

  [[nodiscard]] static std::vector<memory_type>
  compute_nodes_memory_usage_input(Graph<graph_input_type> const &graph,
                                   bool include_parameters = true);


  template <class T>
  memory_type
  compute_memory_usage(Graph<T> const &graph,
                       bool            include_parameters = true) const
  {
    memory_type res = 0;
    auto        nodes_memory_usage =
      compute_nodes_memory_usage(graph, include_parameters);

    res = std::reduce(nodes_memory_usage.cbegin(), nodes_memory_usage.cend());

    return res;
  }


  template <class T>
  memory_type
  compute_memory_usage_input(Graph<T> const &graph,
                             bool            include_parameters = true) const
  {
    memory_type res = 0;
    auto        nodes_memory_usage =
      compute_nodes_memory_usage_input(graph, include_parameters);

    res = std::reduce(nodes_memory_usage.cbegin(), nodes_memory_usage.cend());

    return res;
  }
};



#endif // NETWORK_BUTCHER_COMPUTER_H
