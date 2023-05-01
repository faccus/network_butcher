//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_BUTCHER_H
#define NETWORK_BUTCHER_BUTCHER_H

#include <algorithm>
#include <forward_list>
#include <memory>

#include "Graph_traits.h"

#include "KFinder_Factory.h"

#include "Computer_memory.h"
#include "Parameters.h"
#include "Paths.h"
#include "Utilities.h"
#include "Weight_importers.h"

#include "Constrained_Block_Graph_Builder.h"
#include "Path_Converter.h"

namespace network_butcher
{

  /// Butcher butchers a given graph into slices
  /// \tparam GraphType The type of the graph
  template <typename GraphType>
  class Butcher
  {
  public:
    using network     = GraphType;
    using new_network = block_graph_type;

  private:
    network graph;


    /// Helper function used to estimate the memory usage of a group of nodes
    /// \param devices The devices
    /// \param constraint_type The type of the memory constraint
    /// \param ids The set of nodes to "analyze"
    /// \param input_memory The memory usage of all input nodes
    /// \param output_memory The memory usage of all output nodes
    /// \param params_memory The memory usage of all parameters nodes
    /// \return The pair of maximum memory of ios and of memory of parameters
    [[nodiscard]] std::tuple<memory_type, memory_type>
    estimate_maximum_memory_usage(std::vector<network_butcher::parameters::Device> const &devices,
                                  network_butcher::parameters::Memory_Constraint_Type     constraint_type,
                                  std::set<node_id_type> const                           &ids,
                                  std::vector<memory_type> const                         &input_memory,
                                  std::vector<memory_type> const                         &output_memory,
                                  std::vector<memory_type> const                         &params_memory) const;


    /// Removes the "unfeasible" paths due to memory constraints
    /// \param devices The set of devices
    /// \param new_graph The linearized graph
    /// \param constraint_type The memory constraint
    void
    remove_unfeasible_paths(std::vector<network_butcher::parameters::Device> const &devices,
                            new_network                                            &new_graph,
                            network_butcher::parameters::Memory_Constraint_Type     constraint_type) const;

  public:
    Butcher() = default;

    explicit Butcher(network &&g)
      : graph(std::move(g)){};

    explicit Butcher(network const &g)
      : graph(g){};

    Butcher(Butcher const &) = delete;

    Butcher
    operator=(Butcher const &) = delete;

    Butcher(Butcher &&d) noexcept = default;

    Butcher &
    operator=(Butcher &&d) noexcept = default;


    /// Basic getter for graph
    /// \return The graph (const reference)
    network const &
    get_graph() const
    {
      return graph;
    }


    /// Basic getter (though simple reference) for graph
    /// \return Reference to the graph
    network &
    get_graph_ref()
    {
      return graph;
    }


    network_butcher::types::Weighted_Real_Paths
    compute_k_shortest_path(
      std::function<weight_type(node_id_type const &, std::size_t, std::size_t)> const &transmission_weights,
      network_butcher::parameters::Parameters const                                    &params,
      std::vector<std::unique_ptr<constraints::Extra_Constraint>> const                &extra_constraints = {}) const;
  };


  template <class GraphType>
  std::tuple<memory_type, memory_type>
  Butcher<GraphType>::estimate_maximum_memory_usage(const std::vector<network_butcher::parameters::Device> &devices,
                                                    network_butcher::parameters::Memory_Constraint_Type constraint_type,
                                                    const std::set<node_id_type>                       &ids,
                                                    const std::vector<memory_type>                     &input_memory,
                                                    const std::vector<memory_type>                     &output_memory,
                                                    const std::vector<memory_type> &params_memory) const
  {
    memory_type result_memory = 0, fixed_memory = 0;

    if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
      {
        fixed_memory = std::reduce(std::next(params_memory.begin(), *ids.cbegin()),
                                   std::next(params_memory.begin(), *ids.crbegin()));
      }
    auto const &dependencies = graph.get_neighbors();
    std::size_t qty          = 1;


    if (dependencies[*ids.begin()].first.size() == 1)
      {
        auto const &father = *dependencies[*ids.begin()].first.begin();
        qty                = std::max(qty, dependencies[father].second.size());
      }

    for (auto const &id : ids)
      {
        auto const &node = graph[id];

        auto const &node_dependencies = dependencies[id];
        auto const &parents           = node_dependencies.first;
        auto const &children          = node_dependencies.second;

        memory_type        in_memory  = input_memory[id];
        memory_type const &out_memory = output_memory[id];

        if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
          {
            result_memory = std::max(result_memory, in_memory + out_memory);
          }
        else if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Max)
          {
            result_memory = std::max(result_memory, in_memory + out_memory + params_memory[id]);
          }

        if (children.size() > parents.size())
          qty = qty + children.size() - parents.size();
      }

    return {result_memory * qty, fixed_memory};
  }


  template <class GraphType>
  void
  Butcher<GraphType>::remove_unfeasible_paths(const std::vector<network_butcher::parameters::Device> &devices,
                                              Butcher::new_network                                   &new_graph,
                                              network_butcher::parameters::Memory_Constraint_Type constraint_type) const
  {
    if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::None)
      return;

    std::set<node_id_type> to_remove;


    auto const input_memory  = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_input(graph);
    auto const output_memory = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_output(graph);
    auto const params_memory = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_parameters(graph);

    std::vector<bool> available(devices.size(), true);
    memory_type       memory_graph = 0;


    auto const dependencies_clear = [&](node_id_type const &node_id) {
      auto &dep = new_graph.get_neighbors_ref()[node_id];

      for (auto const &in : dep.first)
        new_graph.get_neighbors_ref()[in].second.erase(node_id);
      for (auto const &out : dep.second)
        new_graph.get_neighbors_ref()[out].first.erase(node_id);

      dep.first.clear();
      dep.second.clear();
    };

    auto const response_fun_max = [&](std::size_t basic_node_id, memory_type const &memory_node) {
      if (memory_graph < memory_node)
        {
          memory_graph = std::max(memory_graph, memory_node);

          for (std::size_t k = 0; k < devices.size(); ++k)
            {
              if (available[k] && memory_graph < devices[k].maximum_memory)
                {
                  continue;
                }
              else
                {
                  available[k] = false;
                  dependencies_clear(basic_node_id + k);
                  to_remove.insert(basic_node_id + k);
                }
            }
        }
    };

    auto const response_fun_preload_parameters =
      [&](std::size_t basic_node_id, memory_type const &param, memory_type const &io) {
        memory_graph += param;

        for (std::size_t k = 0; k < devices.size(); ++k)
          {
            if (available[k] && (memory_graph + io) < devices[k].maximum_memory)
              {
                continue;
              }
            else
              {
                available[k] = false;
                dependencies_clear(basic_node_id + k);
                to_remove.insert(basic_node_id + k);
              }
          }
      };

    for (std::size_t i = 1; i < new_graph.size() - 1; i += devices.size())
      {
        auto const &new_node_content = *new_graph[i].content.second;
        bool        easy_content     = new_node_content.size() == 1;

        if (easy_content)
          {
            auto const index = *new_node_content.begin();

            if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Max)
              {
                response_fun_max(i, params_memory[index] + input_memory[index] + output_memory[index]);
              }
            else if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
              {
                response_fun_preload_parameters(i, params_memory[index], input_memory[index] + output_memory[index]);
              }
          }
        else
          {
            auto const [io_mem, param_mem] = estimate_maximum_memory_usage(
              devices, constraint_type, new_node_content, input_memory, output_memory, params_memory);

            if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Max)
              {
                response_fun_max(i, param_mem + io_mem);
              }
            else if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
              {
                response_fun_preload_parameters(i, param_mem, io_mem);
              }
          }
      }

    new_graph.remove_nodes(to_remove);
  }


  template <class GraphType>
  network_butcher::types::Weighted_Real_Paths
  Butcher<GraphType>::compute_k_shortest_path(
    const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &transmission_weights,
    const network_butcher::parameters::Parameters                                    &params,
    std::vector<std::unique_ptr<constraints::Extra_Constraint>> const                &extra_constraints) const
  {
    using namespace network_butcher::kfinder;

    Constrained_Block_Graph_Builder builder(graph, params, constraints::generate_constraint_function(params, graph));
    builder.construct_weights(transmission_weights);

    for (auto &constraint : extra_constraints)
      builder.add_constraint(constraint->copy());

    auto const new_graph = builder.construct_block_graph();

    auto kFinder = KFinder_Factory<new_network>::Instance().create(params.method, new_graph);

    auto const res = kFinder->compute(params.K);

    network_butcher::Utilities::Path_Converter converter(new_graph);
    return converter.convert_to_weighted_real_path(res);
  }

} // namespace network_butcher
#endif // NETWORK_BUTCHER_BUTCHER_H