//
// Created by faccus on 24/04/23.
//

#ifndef NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H
#define NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H

#include "computer_memory.h"
#include "graph_traits.h"
#include "parameters.h"

namespace network_butcher::constraints
{
  /// A simple class representing a constraint to be applied during the block graph construction
  class Graph_Constraint
  {
  public:
    Graph_Constraint() = default;

    /// Apply the specified constraint to the graph
    /// \param graph
    virtual void
    apply_constraint(Block_Graph_Type &graph) const = 0;

    /// Create a copy of the current constraint (it must be specialized by children classes)
    /// \return A unique pointer to the constructed copy
    [[nodiscard]] virtual auto
    copy() const -> std::unique_ptr<Graph_Constraint>
    {
      return nullptr;
    }

    virtual ~Graph_Constraint() = default;
  };


  /// Simple class used to represent a memory constraint
  /// \tparam GraphType The original graph type
  template <typename GraphType>
  class Memory_Constraint : public Graph_Constraint
  {
  private:
    parameters::Parameters const &params;
    GraphType const              &graph;

    /// Helper function used to estimate the memory usage of a group of nodes
    /// \param devices The devices
    /// \param constraint_type The type of the memory constraint
    /// \param ids The set of nodes to "analyze"
    /// \param input_memory The memory usage of all input nodes
    /// \param output_memory The memory usage of all output nodes
    /// \param params_memory The memory usage of all parameters nodes
    /// \return The pair of maximum memory of ios and of memory of parameters
    [[nodiscard]] auto
    estimate_maximum_memory_usage(const std::vector<network_butcher::parameters::Device> &devices,
                                  network_butcher::parameters::Memory_Constraint_Type     constraint_type,
                                  const std::set<Node_Id_Type>                           &ids,
                                  const std::vector<Memory_Type>                         &input_memory,
                                  const std::vector<Memory_Type>                         &output_memory,
                                  const std::vector<Memory_Type>                         &params_memory) const
      -> std::tuple<Memory_Type, Memory_Type>;

  public:
    /// Constructor
    /// \param params The parameters
    /// \param graph The original graph
    explicit Memory_Constraint(parameters::Parameters const &params, GraphType const &graph)
      : Graph_Constraint()
      , params{params}
      , graph{graph} {};

    /// Removes the "unfeasible" paths due to memory constraints from the block graph
    /// \param graph The block graph
    void
    apply_constraint(Block_Graph_Type &graph) const override;

    /// Create a copy of the current constraint
    /// \return A unique pointer to the constructed copy
    [[nodiscard]] auto
    copy() const -> std::unique_ptr<Graph_Constraint> override;


    ~Memory_Constraint() override = default;
  };

  template <typename GraphType>
  auto
  Memory_Constraint<GraphType>::copy() const -> std::unique_ptr<Graph_Constraint>
  {
    return std::make_unique<Memory_Constraint<GraphType>>(*this);
  }

  template <typename GraphType>
  void
  Memory_Constraint<GraphType>::apply_constraint(Block_Graph_Type &new_graph) const
  {
    auto const &constraint_type = params.block_graph_generation_params.memory_constraint_type;

    if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::None)
      return;

    auto const &devices = params.devices;

    auto const input_memory  = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_input(graph);
    auto const output_memory = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_output(graph);
    auto const params_memory = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_parameters(graph);

    std::vector<bool> available(devices.size(), true);
    Memory_Type       memory_graph = 0;

    // Eliminate dependencies of the specified node
    auto const dependencies_clear = [&](Node_Id_Type const &node_id) {
      auto &dep = new_graph.get_neighbors_ref()[node_id];

      for (auto const &in : dep.first)
        new_graph.get_neighbors_ref()[in].second.erase(node_id);
      for (auto const &out : dep.second)
        new_graph.get_neighbors_ref()[out].first.erase(node_id);

      dep.first.clear();
      dep.second.clear();
    };

    // If Memory_Constraint_Type::Max, then...
    auto const response_fun_max = [&](std::size_t basic_node_id, Memory_Type const &memory_node) {
      // Check if the memory_node is bigger than memory_graph
      if (memory_graph < memory_node)
        {
          // Adjust the memory graph
          memory_graph = std::max(memory_graph, memory_node);

          // Cycle though the devices
          for (std::size_t k = 0; k < devices.size(); ++k)
            {
              // Check if everything fits in memory
              if (available[k] && memory_graph >= devices[k].maximum_memory)
                {
                  // If not, delete the node
                  available[k] = false;
                  dependencies_clear(basic_node_id + k);
                }
            }
        }
    };

    // If Memory_Constraint_Type::Preload_Parameters, then...
    auto const response_fun_preload_parameters =
      [&](std::size_t basic_node_id, Memory_Type const &param, Memory_Type const &io) {
        memory_graph += param;

        for (std::size_t k = 0; k < devices.size(); ++k)
          {
            // Check if everything fits in memory
            if (available[k] && (memory_graph + io) >= devices[k].maximum_memory)
              {
                // If not, delete the node
                available[k] = false;
                dependencies_clear(basic_node_id + k);
              }
          }
      };

    for (std::size_t i = 1; i < new_graph.size() - 1; i += devices.size())
      {
        auto const &new_node_content = *new_graph[i].content.second;

        // If the node corresponds to a single node...
        if (new_node_content.size() == 1)
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
            // Get the IO memory usage and the parameters memory usage
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
  }

  template <typename GraphType>
  auto
  Memory_Constraint<GraphType>::estimate_maximum_memory_usage(
    const std::vector<network_butcher::parameters::Device> &devices,
    network_butcher::parameters::Memory_Constraint_Type     constraint_type,
    const std::set<Node_Id_Type>                           &ids,
    const std::vector<Memory_Type>                         &input_memory,
    const std::vector<Memory_Type>                         &output_memory,
    const std::vector<Memory_Type>                         &params_memory) const -> std::tuple<Memory_Type, Memory_Type>
  {
    Memory_Type result_memory = 0, fixed_memory = 0;

    if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
      {
        fixed_memory = std::reduce(std::next(params_memory.begin(), *ids.cbegin()),
                                   std::next(params_memory.begin(), *ids.crbegin()));
      }
    std::size_t qty = 1;

    if (graph.get_input_nodes(*ids.begin()).size() == 1)
      {
        auto const &father = *graph.get_input_nodes(*ids.begin()).begin();
        qty                = std::max(qty, graph.get_output_nodes(father).size());
      }

    for (auto const &id : ids)
      {
        auto const &node = graph[id];

        auto const &parents  = graph.get_input_nodes(id);
        auto const &children = graph.get_output_nodes(id);

        Memory_Type        in_memory  = input_memory[id];
        Memory_Type const &out_memory = output_memory[id];

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

  /// Simple function to generate a set of constraints from the given constraints
  /// \tparam GraphType The graph type
  /// \param params The parameters of the program
  /// \param graph The original graph
  /// \return The generator function
  template <typename GraphType>
  auto
  generate_constraint_function(parameters::Parameters const &params, GraphType const &graph)
    -> std::function<std::vector<std::unique_ptr<constraints::Graph_Constraint>>()>
  {
    return [&params, &graph]() {
      std::vector<std::unique_ptr<Graph_Constraint>> res;

      if (params.block_graph_generation_params.memory_constraint)
        {
          res.push_back(std::make_unique<Memory_Constraint<GraphType>>(params, graph));
        }

      return res;
    };
  };
} // namespace network_butcher::constraints

#endif // NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H
