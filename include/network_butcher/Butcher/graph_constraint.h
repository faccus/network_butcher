#ifndef NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H
#define NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H

#include <network_butcher/Computer/computer_memory.h>
#include <network_butcher/Network/graph_traits.h>
#include <network_butcher/Types/parameters.h>

namespace network_butcher::constraints
{
  /// A simple class representing a constraint to be applied during the block graph construction
  class Graph_Constraint
  {
  public:
    /// Default constructor
    Graph_Constraint() = default;

    /// Apply the specified constraint to the graph
    /// \param graph The block graph onto which the constraint should be applied
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


  /// Simple class used to represent a memory constraint. It will work properly if connections from device i to device j
  /// are allowed only if i <= j
  /// \tparam GraphType The original graph type
  template <typename GraphType>
  class Memory_Constraint : public Graph_Constraint
  {
  private:
    /// The parameters
    parameters::Parameters const &params;

    /// The original graph. Used to measure the memory usage of each layer of the model
    GraphType const &graph;

    /// Helper function used to estimate the memory usage of a group of nodes
    /// \param ids The set of nodes to "analyze"
    /// \param input_memory The memory usage of all input nodes
    /// \param output_memory The memory usage of all output nodes
    /// \param params_memory The memory usage of all parameters nodes
    /// \return The pair of maximum memory of ios and of memory of parameters
    [[nodiscard]] auto
    estimate_maximum_memory_usage(const std::set<Node_Id_Type>   &ids,
                                  const std::vector<Memory_Type> &input_memory,
                                  const std::vector<Memory_Type> &output_memory,
                                  const std::vector<Memory_Type> &params_memory) const
      -> std::tuple<Memory_Type, Memory_Type>;

    /// It will check if the constraint is applicable to the current graph
    /// \return A pair containing a bool (true if the constraint is applicable) and a string (the reason why it is not
    /// applicable)
    [[nodiscard]] auto
    check_if_applicable() const -> std::pair<bool, std::string>;

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
  auto
  Memory_Constraint<GraphType>::check_if_applicable() const -> std::pair<bool, std::string>
  {
    if (!params.block_graph_generation_params.use_bandwidth_to_manage_connections)
      {
        return {false,
                "Memory_Constraint: The bandwidth is not used to manage the connections. To apply the constraint, "
                "connections from device id i to device  id j must be allowed only if i <= j."};
      }

    // Check if a connection from device j to device i is allowed
    for (std::size_t i = 0; i < params.devices.size(); ++i)
      {
        for (std::size_t j = i + 1; j < params.devices.size(); ++j)
          {
            if (params.weights_params.bandwidth->get_output_nodes(j).contains(i))
              {
                return {false,
                        "Memory_Constraint: A connection from device " + std::to_string(j) + " to device " +
                          std::to_string(i) +
                          " is allowed. To apply the constraint, connections from device id i to device id j must be "
                          "allowed only if i <= j."};
              }
          }
      }

    return {true, ""};
  }

  template <typename GraphType>
  void
  Memory_Constraint<GraphType>::apply_constraint(Block_Graph_Type &new_graph) const
  {
    if (auto const [applicable, reason] = check_if_applicable(); !applicable)
      {
        std::cout << reason << " I will ignore the constraint." << std::endl;
        return;
      }

    auto const &devices     = params.devices;
    auto const  num_devices = devices.size();

    auto const input_memory  = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_input(graph);
    auto const output_memory = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_output(graph);
    auto const params_memory = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_parameters(graph);

    std::vector<bool> available(num_devices, true);
    Memory_Type       memory_graph = 0;

    // Eliminate the in-dependencies of the specified node
    auto const dependencies_clear = [&](Node_Id_Type const &node_id) {
      auto &[input_neighbours, output_neighbours] = new_graph.get_neighbors_ref()[node_id];

      for (auto const &in : input_neighbours)
        new_graph.get_neighbors_ref()[in].second.erase(node_id);

      input_neighbours.clear();
    };

    // Helper function used to update the memory usage and, if necessary, deactivate a device
    auto const response_fun_preload_parameters =
      [&](std::size_t basic_node_id, Memory_Type const &param, Memory_Type const &io) {
        memory_graph += param;

        for (std::size_t k = 0; k < num_devices; ++k)
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
            auto const &index = *new_node_content.cbegin();
            response_fun_preload_parameters(i, params_memory[index], input_memory[index] + output_memory[index]);
          }
        else
          {
            // Get the IO memory usage and the parameters memory usage
            auto const &[io_mem, param_mem] =
              estimate_maximum_memory_usage(new_node_content, input_memory, output_memory, params_memory);

            response_fun_preload_parameters(i, param_mem, io_mem);
          }
      }
  }

  template <typename GraphType>
  auto
  Memory_Constraint<GraphType>::estimate_maximum_memory_usage(const std::set<Node_Id_Type>   &ids,
                                                              const std::vector<Memory_Type> &input_memory,
                                                              const std::vector<Memory_Type> &output_memory,
                                                              const std::vector<Memory_Type> &params_memory) const
    -> std::tuple<Memory_Type, Memory_Type>
  {
    Memory_Type result_memory = 0, fixed_memory = 0;

    // This variable will estimate the maximum number of branches detected. The resulting memory will be given by the
    // space occupied by the parameters and the product between qty and the maximum sum between the input and output
    // of each node. The final result will be an overestimation of the memory required by the group of nodes.
    std::size_t qty = 1;

    // Memory required by the parameters
    fixed_memory =
      std::reduce(std::next(params_memory.begin(), *ids.cbegin()), std::next(params_memory.begin(), *ids.crbegin()));

    // Check if a branch is detected
    if (graph.get_input_nodes(*ids.begin()).size() == 1)
      {
        auto const &father = *graph.get_input_nodes(*ids.begin()).begin();
        qty                = std::max(qty, graph.get_output_nodes(father).size());
      }

    for (auto const &id : ids)
      {
        auto const &parents  = graph.get_input_nodes(id);
        auto const &children = graph.get_output_nodes(id);

        Memory_Type const &in_memory  = input_memory[id];
        Memory_Type const &out_memory = output_memory[id];

        // Compute the maximum memory required by any node
        result_memory = std::max(result_memory, in_memory + out_memory);

        // Adjust qty if extra branches are detected
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
