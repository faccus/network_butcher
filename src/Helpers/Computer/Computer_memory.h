//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_COMPUTER_MEMORY_H
#define NETWORK_BUTCHER_COMPUTER_MEMORY_H

#include "../../Network/WGraph.h"
#include "../Traits/Hardware_traits.h"

class Computer_memory
{
private:
  template <class T>
  [[nodiscard]] static memory_type
  compute_memory_usage_io_collection_type(
    io_collection_type<T> const &collection)
  {
    memory_type res = 0;

    for (auto const &el : collection)
      res += compute_memory_usage(el.second);

    return res;
  }

  template <class T>
  [[nodiscard]] static std::vector<memory_type>
  compute_nodes_memory_usage_gen(
    const Graph<T>                                        &graph,
    std::function<memory_type(Node<T> const &node)> const &func)
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<memory_type> memory_usages;
    memory_usages.resize(nodes.size());

    std::transform(nodes.cbegin(), nodes.cend(), memory_usages.begin(), func);

    return memory_usages;
  }

public:
  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage(const T &in)
  {
    return in.compute_memory_usage();
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage(const std::shared_ptr<T> &in)
  {
    return in->compute_memory_usage();
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage(const std::unique_ptr<T> &in)
  {
    return in->compute_memory_usage();
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage(Content<T> const &in,
                       bool include_initialized = true)
  {
    memory_type res = compute_memory_usage_input(in) + compute_memory_usage_output(in);
    if(include_initialized)
      return res + compute_memory_usage_parameters(in);
    else
      return res;
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_input(Content<T> const &in)
  {
    return compute_memory_usage_io_collection_type(in.get_input());
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_input(Node<Content<T>> const &in)
  {
    return compute_memory_usage_input(in.content);
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_output(Content<T> const &in)
  {
    return compute_memory_usage_io_collection_type(in.get_output());
  }

  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_output(Node<Content<T>> const &in)
  {
    return compute_memory_usage_output(in.content);
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_parameters(Content<T> const &in)
  {
    return compute_memory_usage_io_collection_type(in.get_parameters());
  }

  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_parameters(Node<Content<T>> const &in)
  {
    return compute_memory_usage_parameters(in.content);
  }



  template <class T>
  static inline memory_type
  compute_memory_usage(Node<T> const &node)
  {
    return compute_memory_usage(node.content);
  }

  template <class T>
  static inline memory_type
  compute_memory_usage(Node<Content<T>> const &node,
                       bool include_initialized = true)
  {
    return compute_memory_usage(node.content, include_initialized);
  }



  template <class T>
  static inline std::vector<memory_type>
  compute_nodes_memory_usage(Graph<T> const &graph)
  {
    return compute_nodes_memory_usage_gen(graph, [](Node<T> const &node) {
      return compute_memory_usage(node);
    });
  }

  template <class T>
  static inline std::vector<memory_type>
  compute_nodes_memory_usage(Graph<Content<T>> const &graph,
                             bool include_initialized = true)
  {
    return compute_nodes_memory_usage_gen(
      graph, [include_initialized](Node<T> const &node) {
        return compute_memory_usage(node, include_initialized);
      });
  }

  template <class T>
  static inline memory_type
  compute_memory_usage(Graph<T> const &graph)
  {
    auto const nodes_memory_usage = compute_nodes_memory_usage(graph);

    return std::reduce(nodes_memory_usage.cbegin(), nodes_memory_usage.cend());
  }


  template <class T>
  static inline std::vector<memory_type>
  compute_nodes_memory_usage_input(Graph<Content<T>> const &graph)
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<memory_type> memory_usages;
    memory_usages.resize(nodes.size());

    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](Node<Content<T>> const &node) {
                     return compute_memory_usage_input(node);
                   });
    return memory_usages;
  }

  template <class T>
  static inline std::vector<memory_type>
  compute_nodes_memory_usage_parameters(Graph<Content<T>> const &graph)
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<memory_type> memory_usages;
    memory_usages.resize(nodes.size());

    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](Node<Content<T>> const &node) {
                     return compute_memory_usage_parameters(node);
                   });
    return memory_usages;
  }

  template <class T>
  static inline memory_type
  compute_memory_usage_input(Graph<Content<T>> const &graph)
  {
    auto const nodes_usage = compute_nodes_memory_usage_input(graph);
    return std::reduce(nodes_usage.cbegin(), nodes_usage.cend());
  }

  template <class T>
  static inline memory_type
  compute_memory_usage_parameters(Graph<Content<T>> const &graph)
  {
    auto const nodes_usage = compute_nodes_memory_usage_parameters(graph);
    return std::reduce(nodes_usage.cbegin(), nodes_usage.cend());
  }
};


#endif // NETWORK_BUTCHER_COMPUTER_MEMORY_H
