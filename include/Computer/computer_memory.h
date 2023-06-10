#ifndef NETWORK_BUTCHER_COMPUTER_MEMORY_H
#define NETWORK_BUTCHER_COMPUTER_MEMORY_H

#include <functional>

#include "utilities.h"
#include "wgraph.h"

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
  [[nodiscard]] auto
  compute_memory_usage(const T &in) -> Memory_Type
  {
    return in.compute_memory_usage();
  }

  template <class T>
  [[nodiscard]] auto
  compute_memory_usage(const std::unique_ptr<T> &in) -> Memory_Type
  {
    return in->compute_memory_usage();
  }

  template <class T>
  [[nodiscard]] auto
  compute_memory_usage(const std::shared_ptr<T> &in) -> Memory_Type
  {
    return in->compute_memory_usage();
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage_io_collection_type(Io_Collection_Type<T> const &collection) -> Memory_Type
  {
    Memory_Type res = 0;

    for (auto const &el : collection)
      res += compute_memory_usage(el.second);

    return res;
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage_input(Content_Type<T> const &in) -> Memory_Type
  {
    return compute_memory_usage_io_collection_type(in.get_input());
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage_output(Content_Type<T> const &in) -> Memory_Type
  {
    return compute_memory_usage_io_collection_type(in.get_output());
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage_output(Content_Node_Type<T> const &in) -> Memory_Type
  {
    return compute_memory_usage_output(in.content);
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage_parameters(Content_Type<T> const &in) -> Memory_Type
  {
    return compute_memory_usage_io_collection_type(in.get_parameters());
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage(Content_Type<T> const &in, bool include_initialized = true) -> Memory_Type
  {
    Memory_Type res = compute_memory_usage_input(in) + compute_memory_usage_output(in);
    if (include_initialized)
      return res + compute_memory_usage_parameters(in);
    else
      return res;
  }


  template <class T>
  auto
  compute_memory_usage(Node_Type<T> const &node) -> Memory_Type
  {
    return compute_memory_usage(node.content);
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage_input(Content_Node_Type<T> const &in) -> Memory_Type
  {
    return compute_memory_usage_input(in.content);
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage_parameters(Content_Node_Type<T> const &in) -> Memory_Type
  {
    return compute_memory_usage_parameters(in.content);
  }


  template <class T>
  [[nodiscard]] auto
  compute_memory_usage(Content_Node_Type<T> const &node, bool include_initialized = true) -> Memory_Type
  {
    return compute_memory_usage(node.content, include_initialized);
  }


  template <typename T>
  [[nodiscard]] auto
  compute_nodes_memory_usage_gen(const Contented_Graph_Type<T>                              &graph,
                                 std::function<Memory_Type(Node_Type<T> const &node)> const &func)
    -> std::vector<Memory_Type>
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<Memory_Type> memory_usages(nodes.size());

#if PARALLEL_TBB
    std::transform(std::execution::par, nodes.cbegin(), nodes.cend(), memory_usages.begin(), func);
#else
    #pragma omp parallel default(none) shared(nodes, memory_usages)
    {
      #pragma omp for
      for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          memory_usages[i] = func(nodes[i]);
        }
    }
#endif

    return memory_usages;
  }


  template <typename T>
  [[nodiscard]] auto
  compute_nodes_memory_usage(Contented_Graph_Type<T> const &graph, bool include_initialized = true)
    -> std::vector<Memory_Type>
  {
    return compute_nodes_memory_usage_gen(graph, [include_initialized](Content_Node_Type<T> const &node) {
      return compute_memory_usage(node, include_initialized);
    });
  }


  template <typename T>
  [[nodiscard]] auto
  compute_nodes_memory_usage_input(Contented_Graph_Type<T> const &graph) -> std::vector<Memory_Type>
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<Memory_Type> memory_usages(nodes.size());

#if PARALLEL_TBB
    std::transform(std::execution::par,
                   nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](Content_Node_Type<T> const &node) { return compute_memory_usage_input(node); });
#else
    #pragma omp parallel default(none) shared(nodes, memory_usages)
    {
      #pragma omp for
      for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          memory_usages[i] = compute_memory_usage_input(nodes[i]);
        }
    }
#endif

    return memory_usages;
  }


  template <typename T>
  [[nodiscard]] auto
  compute_nodes_memory_usage_output(Contented_Graph_Type<T> const &graph, bool include_parameters = false)
    -> std::vector<Memory_Type>
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<Memory_Type> memory_usages(nodes.size());

    std::function<Memory_Type(Content_Node_Type<T> const &)> func;

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
    #pragma omp parallel default(none) shared(nodes, memory_usages, func)
    {
      #pragma omp for
      for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          memory_usages[i] = func(nodes[i]);
        }
    }
#endif

    return memory_usages;
  }


  template <typename T>
  [[nodiscard]] auto
  compute_nodes_memory_usage_parameters(Contented_Graph_Type<T> const &graph) -> std::vector<Memory_Type>
  {
    auto const              &nodes = graph.get_nodes();
    std::vector<Memory_Type> memory_usages(nodes.size());

#if PARALLEL_TBB
    std::transform(std::execution::par,
                   nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](Content_Node_Type<T> const &node) { return compute_memory_usage_parameters(node); });
#else

    #pragma omp parallel default(none) shared(nodes, memory_usages)
    {
      #pragma omp for
      for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          memory_usages[i] = compute_memory_usage_parameters(nodes[i]);
        }
    }

#endif

    return memory_usages;
  }


  template <typename T>
  [[nodiscard]] auto
  compute_memory_usage_input(Contented_Graph_Type<T> const &graph) -> Memory_Type
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
  auto
  compute_memory_usage_parameters(Contented_Graph_Type<T> const &graph) -> Memory_Type
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
