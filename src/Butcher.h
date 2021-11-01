//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_BUTCHER_H
#define NETWORK_BUTCHER_BUTCHER_H

#include "Helpers/Computer/Computer_memory.h"
#include "Network/Graph.h"
#include "Network/Node.h"

#include "Helpers/K-shortest_path/KEppstein.h"
#include "Helpers/Traits/Graph_traits.h"
#include "Helpers/Types/Type_info.h"
#include "Helpers/Utilities.h"


/// Butcher butchers a given graph into slices
template <class T>
class Butcher
{
private:
  using network = Graph<T>;
  network graph;

  /// Compute the minimal connected graphs (with dependencies) containing every
  /// node \return returns the smallest connected sub-graph (with dependencies)
  /// that connects the first node with the n-th node
  [[nodiscard]] std::vector<slice_type>
  compute_basic_routes() const
  {
    std::vector<slice_type> basic_routes;
    basic_routes.reserve(graph.nodes.size());

    {
      slice_type tmp;
      tmp.insert(0);
      basic_routes.push_back(tmp);
    }

    for (int i = 1; i < graph.nodes.size(); ++i)
      {
        slice_type partial_res;
        partial_res.insert(i);

        const auto &input_nodes = graph.dependencies[i].first;

        // Construct the current basic_route the i (just add {i} to the basic
        // routes of the parents).
        for (auto &node : input_nodes)
          partial_res.insert(basic_routes[node].begin(),
                             basic_routes[node].end());
        basic_routes.push_back(partial_res);
      }

    return basic_routes;
  };

  /// Given a vector of slices, verify which ones applied to tester return true.
  /// Note that, at the end, all the ok slices will be moved to the return
  /// vector, while the non-compatible ones will be deleted \param slices The
  /// vector of input slices \param tester The tester function \return The
  /// slices that satisfy the tester function
  static std::vector<slice_type>
  partition_checker(std::vector<slice_type>                       &slices,
                    const std::function<bool(const slice_type &)> &tester)
  {
    std::vector<slice_type> res;

    for (int i = 0; i < slices.size(); ++i)
      {
        if (tester(slices[i]))
          res.push_back(std::move(slices[i]));
        else
          {
            {
              slice_type tmp(std::move(slices[i]));
            }
          }
      }

    return res;
  };

  [[nodiscard]] std::pair<Graph<std::set<node_id_type>>,
                          std::function<type_weight(edge_type)>>
  block_graph() const
  {
    std::vector<node_type>                       new_nodes;
    std::map<io_id_type, std::set<node_id_type>> new_content;
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      new_dependencies;

    new_nodes.reserve(graph.nodes.size());
    new_dependencies.reserve(graph.nodes.size());

    int         counter = 0;
    std::size_t id      = 0;
    for (auto &node : graph.nodes)
      {
        bool        done          = false;
        auto const &dep           = graph.dependencies[node.get_id()];
        auto const  local_counter = dep.second.size() - dep.first.size();

        if (local_counter == 0 && counter == 0) // Add new node
          {
            new_nodes.emplace_back(id);
            new_dependencies.emplace_back();
            new_dependencies.back().first =
              graph.dependencies[node.get_id()].first;

            new_content[id].insert(node.get_id());

            for (auto &in : new_dependencies.back().first)
              new_dependencies[in].second.insert(node.get_id());

            ++id;
          }
        else if (local_counter > 0 &&
                 counter ==
                   0) // Add new node and add master node for next steps
          {
            new_nodes.emplace_back(id);
            new_dependencies.emplace_back();
            new_dependencies.back().first =
              graph.dependencies[node.get_id()].first;

            new_content[id].insert(node.get_id());

            for (auto &in : new_dependencies.back().first)
              new_dependencies[in].second.insert(node.get_id());
            ++id;

            new_nodes.emplace_back(id);
            new_dependencies.emplace_back();
            new_dependencies.back().first.insert(node.get_id());
            new_content[id];
          }
        else if (local_counter == 0 && dep.second.size() == 1 && counter > 0)
          {}
      }

    return {};
  }


public:
  Butcher() = default;
  /// Move constructor
  explicit Butcher(network &&g)
    : graph(std::move(g)){};

  /// Read from a file the model, construct the associated graph and prepare the
  /// butcher
  /// \param p Full path to the .onnx file model
  /// \param ignore_parameters Allows to choose if graph should ignore already
  /// initialized inputs/outputs (parameters)
  Butcher(const std::string &p, bool ignore_parameters = true)
    : graph(p, ignore_parameters){};

  /// It will compute every possible 2-slice partition of the network and it
  /// will select the partition whose total memory usage is less than the
  /// specified value \param memory_first_slice Total memory usage allowed to
  /// the first slice \return a collection of all the admissible partitions (and
  /// the nodes contained in the first partition)
  std::vector<slice_type>
  compute_two_slice_memory_brute_force(memory_type memory_first_slice) const
  {
    Computer_memory computer;
    auto            slices  = compute_two_slice_brute_force();
    auto nodes_memory_usage = computer.compute_nodes_memory_usage_input(graph);

    auto tester = [&nodes_memory_usage,
                   &memory_first_slice](const slice_type &slice) {
      memory_type memory_usage = 0;
      for (auto &j : slice)
        memory_usage += nodes_memory_usage[j];
      return memory_usage < memory_first_slice;
    };

    return partition_checker(slices, tester);
  };

  /// It will try to compute every 2-slice partition of a graph
  /// \return A vector containing every possibile 2-slice partition of the graph
  /// (taking into account dependencies)
  std::vector<slice_type>
  compute_two_slice_brute_force() const
  {
    std::vector<slice_type> res;
    res.reserve(graph.nodes.size());

    slice_type start{0};
    res.push_back(start);

    for (int i = 1; i < graph.nodes.size(); ++i)
      {
        auto &dependencies = graph.dependencies[i];
        auto &input        = dependencies.first;

        for (auto &part : res)
          {
            bool ok = true;
            for (auto &in : input)
              if (!part.contains(in))
                {
                  ok = false;
                  break;
                }
            if (ok)
              {
                auto copy = part;
                copy.insert(i);
                res.push_back(copy);
              }
          }
      }

    return res;
  };

  std::vector<typename KFinder<T>::path_info>
  compute_k_shortest_paths(std::function<type_weight(edge_type)> &weights) const
  {}
};


#endif // NETWORK_BUTCHER_BUTCHER_H
