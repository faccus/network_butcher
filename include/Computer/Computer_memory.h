//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_COMPUTER_MEMORY_H
#define NETWORK_BUTCHER_COMPUTER_MEMORY_H

#include <functional>

#include "Utilities.h"
#include "WGraph.h"

namespace network_butcher::computer::Computer_memory
{
  template <class T>
  using Node_Type = network_butcher::types::CNode<T>;

  template <class T>
  using Content_Type = network_butcher::types::Content<T>;

  template <class T>
  using Content_Node_Type = network_butcher::types::CNode<network_butcher::types::Content<T>>;

  template <class T>
  using Contented_Graph_Type = network_butcher::types::Graph<Content_Node_Type<T>>;


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage(const T &in)
  {
    return in.compute_memory_usage();
  }

  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage(const std::unique_ptr<T> &in)
  {
    return in->compute_memory_usage();
  }

  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage(const std::shared_ptr<T> &in)
  {
    return in->compute_memory_usage();
  }


  template <class T>
  [[nodiscard]] static memory_type
  compute_memory_usage_io_collection_type(io_collection_type<T> const &collection)
  {
    memory_type res = 0;

    for (auto const &el : collection)
      res += compute_memory_usage(el.second);

    return res;
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_input(Content_Type<T> const &in)
  {
    return compute_memory_usage_io_collection_type(in.get_input());
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_output(Content_Type<T> const &in)
  {
    return compute_memory_usage_io_collection_type(in.get_output());
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_output(Content_Node_Type<T> const &in)
  {
    return compute_memory_usage_output(in.content);
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_parameters(Content_Type<T> const &in)
  {
    return compute_memory_usage_io_collection_type(in.get_parameters());
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage(Content_Type<T> const &in, bool include_initialized = true)
  {
    memory_type res = compute_memory_usage_input(in) + compute_memory_usage_output(in);
    if (include_initialized)
      return res + compute_memory_usage_parameters(in);
    else
      return res;
  }


  template <class T>
  static inline memory_type
  compute_memory_usage(Node_Type<T> const &node)
  {
    return compute_memory_usage(node.content);
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_input(Content_Node_Type<T> const &in)
  {
    return compute_memory_usage_input(in.content);
  }


  template <class T>
  [[nodiscard]] inline static memory_type
  compute_memory_usage_parameters(Content_Node_Type<T> const &in)
  {
    return compute_memory_usage_parameters(in.content);
  }


  template <class T>
  static inline memory_type
  compute_memory_usage(Content_Node_Type<T> const &node, bool include_initialized = true)
  {
    return compute_memory_usage(node.content, include_initialized);
  }


  template <typename T>
  [[nodiscard]] static std::vector<memory_type>
  compute_nodes_memory_usage_gen(const Contented_Graph_Type<T>                              &graph,
                                 std::function<memory_type(Node_Type<T> const &node)> const &func)
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<memory_type> memory_usages;
    memory_usages.resize(nodes.size());

#if PARALLEL_TBB
    std::transform(std::execution::par, nodes.cbegin(), nodes.cend(), memory_usages.begin(), func);
#else
#  pragma omp parallel default(none) shared(nodes, memory_usages)
    {
#  pragma omp for
      for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          memory_usages[i] = func(nodes[i]);
        }
    }
#endif

    return memory_usages;
  }


  template <typename T>
  static inline std::vector<memory_type>
  compute_nodes_memory_usage(Contented_Graph_Type<T> const &graph, bool include_initialized = true)
  {
    return compute_nodes_memory_usage_gen(graph, [include_initialized](Content_Node_Type<T> const &node) {
      return compute_memory_usage(node, include_initialized);
    });
  }


  template <typename T>
  static inline std::vector<memory_type>
  compute_nodes_memory_usage_input(Contented_Graph_Type<T> const &graph)
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<memory_type> memory_usages;
    memory_usages.resize(nodes.size());

#if PARALLEL_TBB
    std::transform(std::execution::par,
                   nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](Content_Node_Type<T> const &node) { return compute_memory_usage_input(node); });
#else
#  pragma omp parallel default(none) shared(nodes, memory_usages)
    {
#  pragma omp for
      for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          memory_usages[i] = compute_memory_usage_input(nodes[i]);
        }
    }
#endif

    return memory_usages;
  }


  template <typename T>
  static inline std::vector<memory_type>
  compute_nodes_memory_usage_output(Contented_Graph_Type<T> const &graph, bool include_parameters = false)
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<memory_type> memory_usages;
    memory_usages.resize(nodes.size());

    std::function<memory_type(Content_Node_Type<T> const &)> func;

    if (include_parameters)
      {
        func = [](Content_Node_Type<T> const &node) {
          return compute_memory_usage_output(node) + compute_memory_usage_parameters(node);
        };
      }
    else
      {
        func = [](Content_Node_Type<T> const &node) { return compute_memory_usage_output(node); };
      }


#if PARALLEL_TBB
    std::transform(std::execution::par, nodes.cbegin(), nodes.cend(), memory_usages.begin(), func);
#else
#  pragma omp parallel default(none) shared(nodes, memory_usages, func)
    {
#  pragma omp for
      for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          memory_usages[i] = func(nodes[i]);
        }
    }
#endif

    return memory_usages;
  }


  template <typename T>
  static inline std::vector<memory_type>
  compute_nodes_memory_usage_parameters(Contented_Graph_Type<T> const &graph)
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<memory_type> memory_usages;
    memory_usages.resize(nodes.size());

#if PARALLEL_TBB
    std::transform(std::execution::par,
                   nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](Content_Node_Type<T> const &node) { return compute_memory_usage_parameters(node); });
#else

#  pragma omp parallel default(none) shared(nodes, memory_usages)
    {
#  pragma omp for
      for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          memory_usages[i] = compute_memory_usage_parameters(nodes[i]);
        }
    }

#endif

    return memory_usages;
  }


  template <typename T>
  static inline auto
  compute_memory_usage_input(Contented_Graph_Type<T> const &graph)
  {
    auto const nodes_usage = compute_nodes_memory_usage_input(graph);

#if PARALLEL_TBB
    return std::reduce(std::execution::par, nodes_usage.cbegin(), nodes_usage.cend());
#else
    if (nodes_usage.size() == 1)
      {
        return nodes_usage.front();
      }

    auto mem = nodes_usage.front();
    #pragma omp parallel default(none) shared(nodes_usage, mem)
    {
      #pragma omp for reduction(+ : mem)
      for (std::size_t i = 1; i < nodes_usage.size(); ++i)
        {
          mem += nodes_usage[i];
        }
    }

    return mem;
#endif
  }


  template <typename T>
  static inline auto
  compute_memory_usage_parameters(Contented_Graph_Type<T> const &graph)
  {
    auto const nodes_usage = compute_nodes_memory_usage_parameters(graph);

#if PARALLEL_TBB
    return std::reduce(std::execution::par, nodes_usage.cbegin(), nodes_usage.cend());
#else
    if (nodes_usage.size() == 1)
      {
        return nodes_usage.front();
      }

    auto mem = nodes_usage.front();
#pragma omp parallel default(none) shared(nodes_usage, mem)
    {
#pragma omp for reduction(+ : mem)
      for (std::size_t i = 1; i < nodes_usage.size(); ++i)
        {
          mem += nodes_usage[i];
        }
    }

    return mem;
#endif
  }
} // namespace network_butcher::computer::Computer_memory


#endif // NETWORK_BUTCHER_COMPUTER_MEMORY_H
