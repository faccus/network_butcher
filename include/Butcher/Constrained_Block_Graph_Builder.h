//
// Created by faccus on 25/04/23.
//

#ifndef NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H
#define NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H

#include "Block_Graph_Builder.h"

#include "Weight_importers.h"

namespace network_butcher
{
  /// This block graph builder class is used when extra constraints have to be applied during the construction of block
  /// graph
  /// \tparam GraphType The original graph type
  template <typename GraphType>
  class Constrained_Block_Graph_Builder : public Block_Graph_Builder<GraphType>
  {
  protected:
    using Parent = Block_Graph_Builder<GraphType>;

    std::vector<std::unique_ptr<constraints::Extra_Constraint>>                constraints;
    parameters::Parameters const                                              &params;
    bool                                                                       weights;
    std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> transmission_weights;

    /// The actual construction of the block graph is performed when this function is called
    /// \param block_graph_mode The block graph mode (classic, input, output)
    /// \param starting_device_id The id of the device onto which the input is produced
    /// \param ending_device_id The id of the device that needs the final output
    /// \param backward_connections_allowed This is set to false if a connection between a device with id i to a device
    /// with id j is allowed only if i>=j \return The resulting block graph
    block_graph_type
    build_block_graph(parameters::Block_Graph_Generation_Mode block_graph_mode,
                      std::size_t                             starting_device_id,
                      std::size_t                             ending_device_id,
                      bool                                    backward_connections_allowed);

    /// Apply to the input block graph the weights from the original graph
    /// \param new_graph The block graph
    void
    apply_operation_weights(block_graph_type &new_graph);

    /// Apply to the input block graph the transmission weights
    /// \param new_graph The block graph
    void
    apply_transmission_weights(block_graph_type &new_graph);

    /// Apply to the input graph the given collection of constraints
    /// \param new_graph
    void
    apply_constraints(block_graph_type &new_graph);


  public:
    /// Simple constructor for a Constrained Block Graph Builder
    /// \param original_graph The original graph. This is saved through a const reference
    /// \param params The program parameters
    /// \param expression_generator A function that should generate the initial constraints for the builder
    explicit Constrained_Block_Graph_Builder(
      GraphType const                                                                    &original_graph,
      parameters::Parameters const                                                       &params,
      std::function<std::vector<std::unique_ptr<constraints::Extra_Constraint>>()> const &expression_generator =
        nullptr)
      : Block_Graph_Builder<GraphType>(original_graph)
      , params{params}
      , weights{false}
      , transmission_weights(nullptr)
    {
      if (expression_generator)
        {
          constraints = expression_generator();
        }
    };


    /// Call this function if the builder should apply the transmission weights during the block graph construction
    /// \param in_transmission_weights The transmission weights
    void
    construct_transmission_weights(
      std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> const &in_transmission_weights);


    /// Call this function if the builder should apply the weights from the original graph during the block graph
    /// construction
    void
    construct_operation_weights();


    /// Call this function if the builder should apply the transmission weights and the weights from the original graph
    /// during the block graph construction \param in_transmission_weights The transmission weights
    void
    construct_weights(
      std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> const &in_transmission_weights);


    /// Add to the collection of constraints the input constraint
    /// \param constraint A r-value reference to the new constraint
    void
    add_constraint(std::unique_ptr<constraints::Extra_Constraint> &&constraint);


    /// The basic construct method. It will produce the block graph using the specified options.
    /// \return The resulting block graph
    block_graph_type
    construct_block_graph() override;


    ~Constrained_Block_Graph_Builder() override = default;
  };


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::construct_weights(
    const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &in_transmission_weights)
  {
    construct_operation_weights();
    construct_transmission_weights(in_transmission_weights);
  }


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::construct_operation_weights()
  {
    weights = true;
  }


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::construct_transmission_weights(
    const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &in_transmission_weights)
  {
    this->transmission_weights = in_transmission_weights;
  }


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::add_constraint(
    std::unique_ptr<constraints::Extra_Constraint> &&constraint)
  {
    constraints.emplace_back(std::move(constraint));
  };


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::apply_constraints(block_graph_type &new_graph)
  {
    for (auto const &constraint : constraints)
      {
        constraint->apply_constraint(new_graph);
      }
  };


  template <typename GraphType>
  block_graph_type
  Constrained_Block_Graph_Builder<GraphType>::build_block_graph(
    parameters::Block_Graph_Generation_Mode block_graph_mode,
    std::size_t                             starting_device_id,
    std::size_t                             ending_device_id,
    bool                                    backward_connections_allowed)
  {
    // It will construct the linearized version of the original graph
    auto const linearize_graph = [](GraphType const                                                &old_graph,
                                    network_butcher::parameters::Block_Graph_Generation_Mode const &mode) {
      auto const &old_nodes = old_graph.get_nodes();

      // Counter is used to establish if the current node has more output
      // connections than the inputs one.
      int counter = old_graph.get_output_nodes(0).size() - old_graph.get_input_nodes(0).size() - 1;

      std::list<block_graph_type::Node_Type> starting_nodes;


      starting_nodes.emplace_back(block_graph_type::Node_Type::Content_Type{0, nullptr});
      starting_nodes.back().content.second = std::make_shared<node_id_collection_type>(node_id_collection_type{0});

      // Cycle through all the nodes of the graph
      for (auto it = ++old_nodes.begin(); it != old_nodes.end(); ++it)
        {
          // Node of the old graph
          auto const &node        = *it;
          auto const &input_deps  = old_graph.get_input_nodes(node.get_id());
          auto const &output_deps = old_graph.get_output_nodes(node.get_id());

          int const local_counter = output_deps.size() - input_deps.size();

          // Add new node
          if (local_counter <= 0 && counter == 0)
            {
              starting_nodes.emplace_back(block_graph_type::Node_Type::Content_Type{0, nullptr});
              starting_nodes.back().content.second =
                std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});
            }
          // Add new node and add master node for next steps
          else if (local_counter > 0 && counter == 0)
            {
              starting_nodes.emplace_back(block_graph_type::Node_Type::Content_Type{0, nullptr});
              starting_nodes.back().content.second =
                std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});

              starting_nodes.emplace_back(block_graph_type::Node_Type::Content_Type{0, nullptr});
              starting_nodes.back().content.second = std::make_shared<node_id_collection_type>();

              counter += local_counter;
            }
          // Add node link to the "big" node
          else if ((local_counter == 0 && output_deps.size() == 1 || local_counter > 0 && input_deps.size() <= 1) &&
                   counter > 0)
            {
              starting_nodes.back().content.second->insert(starting_nodes.back().content.second->end(), node.get_id());

              counter += local_counter;
            }
          else if (counter > 0 && ((local_counter >= 0 && input_deps.size() > 1) || (local_counter < 0)))
            {
              counter -= (input_deps.size() - 1);

              // End of the master node
              if (counter == 0)
                {
                  starting_nodes.emplace_back(block_graph_type::Node_Type::Content_Type{0, nullptr});
                  starting_nodes.back().content.second =
                    std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});

                  // Do we have to add another master node?
                  if (local_counter >= 0)
                    {
                      starting_nodes.emplace_back(block_graph_type::Node_Type::Content_Type{0, nullptr});
                      starting_nodes.back().content.second = std::make_shared<node_id_collection_type>();
                    }
                }
              else
                {
                  starting_nodes.back().content.second->insert(starting_nodes.back().content.second->end(),
                                                               node.get_id());
                }

              counter += (output_deps.size() - 1);
            }
          else
            {
              throw std::runtime_error("Unknown node found during block_graph construction");
            }
        }

      // If the block graph mode is not setted to classic, then we need to merge either the input or the output nodes of
      // the "big" nodes
      if (starting_nodes.size() > 1 && mode != network_butcher::parameters::Block_Graph_Generation_Mode::classic)
        {
          // Simple lambda to be used to merge the nodes contained in the two specified collections. It will also change
          // the two original iterators. Returns false if the new iterator is the end
          auto const merge_nodes = [&starting_nodes](auto &it_succ, auto &it_prec) {
            auto &it_nodes_edit = it_succ->content.second;

            it_prec->content.second->insert(std::make_move_iterator(it_nodes_edit->begin()),
                                            std::make_move_iterator(it_nodes_edit->end()));

            starting_nodes.erase(it_succ);
            it_succ = it_prec;
            ++it_succ;

            if (it_succ == starting_nodes.cend())
              return true;

            return false;
          };

          if (mode == network_butcher::parameters::Block_Graph_Generation_Mode::input)
            {
              // Loops through the starting nodes looking for a "big" node
              for (auto it_succ = ++starting_nodes.begin(), it_prec = starting_nodes.begin();
                   it_succ != starting_nodes.end();
                   ++it_succ, ++it_prec)
                {
                  auto const &it_prec_nodes_const = it_prec->content.second;

                  // If we detect that it_prec points to the input of a big node....
                  if (it_prec_nodes_const->size() == 1 &&
                      old_graph.get_output_nodes(*it_prec_nodes_const->cbegin()).size() > 1)
                    {
                      // It will merge them
                      if (merge_nodes(it_succ, it_prec))
                        break;
                    }
                }
            }
          else if (mode == network_butcher::parameters::Block_Graph_Generation_Mode::output)
            {
              // Loops through the starting nodes looking for a "big" node
              for (auto it_succ = ++starting_nodes.begin(), it_prec = starting_nodes.begin();
                   it_succ != starting_nodes.end();
                   ++it_succ, ++it_prec)
                {
                  auto const &it_nodes_const = it_succ->content.second;

                  // If we detect that it_succ points to the output of a big node....
                  if (it_nodes_const->size() == 1 && old_graph.get_input_nodes(*it_nodes_const->cbegin()).size() > 1)
                    {
                      // It will merge them
                      if (merge_nodes(it_succ, it_prec))
                        break;
                    }
                }
            }
        }

      return starting_nodes;
    };

    // It will add all the nodes required in the block graph
    auto const add_extra_nodes_per_device = [](std::list<block_graph_type::Node_Type> &starting_nodes,
                                               std::size_t                             num_devices) {
      if (starting_nodes.size() > 2)
        {
          for (auto it_follower = ++starting_nodes.cbegin(), it = ++(++starting_nodes.cbegin());
               it != starting_nodes.cend();
               it_follower = it, ++it)
            {
              for (std::size_t i = 1; i < num_devices; ++i)
                {
                  starting_nodes.emplace(it, block_graph_type::Node_Type::Content_Type{i, it_follower->content.second});
                }
            }
        }
    };

    // It will produce the dependencies for the block graph
    auto const process_dependencies =
      [](std::size_t dep_size, std::size_t supp_size, std::size_t num_devices, bool backward_allowed) {
        block_graph_type::Dependencies_Type new_dependencies;
        new_dependencies.reserve(dep_size);

        // The first node is fully connected with the first layer
        {
          new_dependencies.emplace_back();

          auto &out = new_dependencies.back().second;
          for (std::size_t k = 0; k < num_devices; ++k)
            out.insert(out.end(), k + 1);
        }

        // Inputs: first node, Outputs: following layer nodes
        {
          new_dependencies.emplace_back(std::make_pair<node_id_collection_type, node_id_collection_type>({0}, {}));
          auto &out = new_dependencies.back().second;
          for (std::size_t k = 0; k < num_devices; ++k)
            out.insert(out.end(), 1 + num_devices + k);

          for (std::size_t k = 1; k < num_devices; ++k)
            {
              auto dep_cpy = new_dependencies.back();

              if (!backward_allowed)
                dep_cpy.second.erase(num_devices + k);

              new_dependencies.push_back(std::move(dep_cpy));
            }
        }

        // Inputs: previous layer nodes, Outputs: following layer nodes
        {
          for (std::size_t i = 2; i < supp_size; ++i)
            {
              auto const id = new_dependencies.size();
              new_dependencies.emplace_back(std::make_pair<node_id_collection_type, node_id_collection_type>({}, {}));

              auto &in  = new_dependencies.back().first;
              auto &out = new_dependencies.back().second;

              if (!backward_allowed)
                in.insert(in.end(), id - num_devices);


              for (std::size_t k = 0; k < num_devices; ++k)
                {
                  if (backward_allowed)
                    in.insert(in.end(), id - num_devices + k);

                  out.insert(out.end(), id + num_devices + k);
                }

              for (std::size_t k = 1; k < num_devices; ++k)
                {
                  auto tmp_dep = new_dependencies.back();

                  if (!backward_allowed)
                    {
                      tmp_dep.first.insert(id - num_devices + k);
                      tmp_dep.second.erase(id + num_devices + k - 1);
                    }

                  new_dependencies.emplace_back(std::move(tmp_dep));
                }
            }
        }

        // Inputs: previous layer nodes, Outputs: last node
        {
          auto const id = new_dependencies.size();

          new_dependencies.emplace_back(
            std::make_pair<node_id_collection_type, node_id_collection_type>({}, {id + num_devices}));

          auto &in = new_dependencies.back().first;
          in.insert(in.end(), id - num_devices);

          if (backward_allowed)
            for (std::size_t k = 1; k < num_devices; ++k)
              in.insert(in.end(), id - num_devices + k);

          for (std::size_t k = 1; k < num_devices; ++k)
            {
              auto dep_cpy = new_dependencies.back();

              if (!backward_allowed)
                dep_cpy.first.insert(id - num_devices + k);

              new_dependencies.emplace_back(std::move(dep_cpy));
            }
        }

        // The last layer is fully connected with the last node
        {
          new_dependencies.emplace_back(std::make_pair<node_id_collection_type, node_id_collection_type>({}, {}));

          auto &in = new_dependencies.back().first;
          for (std::size_t k = 0; k < num_devices; ++k)
            in.insert(in.end(), new_dependencies.size() - 1 - num_devices + k);
        }

        return new_dependencies;
      };

    // Get the linearized graph
    auto starting_nodes = linearize_graph(this->original_graph, block_graph_mode);

    auto const supp_size = starting_nodes.size() - 2;
    // Add the required nodes to the collection of nodes
    add_extra_nodes_per_device(starting_nodes, this->original_graph.get_num_devices());

    // Prepare the collection of nodes
    block_graph_type::Node_Collection_Type new_nodes;
    new_nodes.reserve(starting_nodes.size());

    new_nodes.insert(new_nodes.end(),
                     std::make_move_iterator(starting_nodes.begin()),
                     std::make_move_iterator(starting_nodes.end()));

    // Fix the input/output device ids
    new_nodes.front().content.first = starting_device_id;
    new_nodes.back().content.first  = ending_device_id;

    return block_graph_type(new_nodes,
                            process_dependencies(new_nodes.size(),
                                                 supp_size,
                                                 this->original_graph.get_num_devices(),
                                                 params.backward_connections_allowed));
  }


  template <typename GraphType>
  block_graph_type
  Constrained_Block_Graph_Builder<GraphType>::construct_block_graph()
  {
    // Construct the "naked" block graph
    auto new_graph = build_block_graph(params.block_graph_mode,
                                       params.starting_device_id,
                                       params.ending_device_id,
                                       params.backward_connections_allowed);

    // Apply weights from the original graph
    if (weights)
      {
        apply_operation_weights(new_graph);
      }

    // Apply transmission weights
    if (transmission_weights != nullptr)
      {
        apply_transmission_weights(new_graph);
      }

    // Apply the constraints
    apply_constraints(new_graph);

    return new_graph;
  };


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::apply_operation_weights(block_graph_type &new_graph)
  {
    using namespace network_butcher::parameters;

    auto const &nodes          = new_graph.get_nodes();
    auto const &weights_params = params.weights_params;

    // Check if we have to generate the weights though aMLLibrary
    if (weights_params.weight_import_mode == Weight_Import_Mode::aMLLibrary_block)
      {
        if constexpr (std::is_same_v<GraphType, graph_type>)
          {
            io::block_aMLLibrary_Weight_Importer(this->original_graph, new_graph, params).import_weights();
          }
        else
          {
            throw std::runtime_error("The specified weight import mode is not supported!");
          }
      }
    // Check if we have to import the block graph weights directly from a .csv file
    else if (weights_params.weight_import_mode == Weight_Import_Mode::block_single_direct_read)
      {
        io::Csv_Weight_Importer<block_graph_type>(new_graph,
                                                  {weights_params.single_weight_import_path},
                                                  weights_params.single_csv_columns_weights,
                                                  params.devices,
                                                  weights_params.separator)
          .import_weights();
      }
    // Check if we have to import the block graph weights directly from multiple .csv files
    else if (weights_params.weight_import_mode == Weight_Import_Mode::block_multiple_direct_read)
      {
        std::vector<std::string> paths, entries;
        for (auto const &device : params.devices)
          {
            paths.push_back(device.weights_path);
            entries.push_back(device.relevant_entry);
          }

        io::Csv_Weight_Importer<block_graph_type>(new_graph, paths, entries, params.devices, weights_params.separator)
          .import_weights();
      }
    // Standard weight import
    else
      {
        auto const add_weight_cost = [&new_graph, &graph = this->original_graph, mode = params.block_graph_mode](
                                       block_graph_type::Node_Type const &node) {
          auto const first = node.get_id();

          // Look for the nodes of the original graph that are
          // represented by the input node (in the linearized
          // graph)
          auto const &inputs = *node.content.second;

          // The device id of the input node (=0 starting device, >0 other device)
          auto const in_device_id = node.content.first;

          for (auto const &second : new_graph.get_output_nodes(first))
            {
              auto const &out_node = new_graph[second];

              edge_type const edge = {first, second};
              // The device id of the output node (=0 starting device, >0 other device)
              auto const out_device_id = out_node.content.first;

              // Look for the nodes of the original graph that are represented by the output node (in the
              // linearized graph)
              auto const &outputs = *out_node.content.second;

              double weight_cost = 0.;

              // 1-1 correspondence
              if (outputs.size() == 1 && inputs.size() == 1)
                {
                  auto const &input  = *inputs.begin();
                  auto const &output = *outputs.begin();

                  auto const tmp_edge = std::make_pair(input, output);

                  weight_cost = graph.get_weight(out_device_id, tmp_edge);
                }
              // (2+)-1 correspondence
              else if (outputs.size() == 1)
                {
                  auto const &output           = *outputs.begin();
                  auto const &inputs_of_output = graph.get_input_nodes(output);

                  weight_cost = graph.get_weight(out_device_id, std::make_pair(*inputs_of_output.cbegin(), output));
                }
              // 1-(2+) correspondence
              else if (inputs.size() == 1)
                {
                  auto const &input             = *inputs.begin();
                  auto const &interface_outputs = graph.get_output_nodes(input);

                  for (auto const &output : interface_outputs)
                    weight_cost += graph.get_weight(out_device_id, std::make_pair(input, output));

                  // Compute the total weight associated to the internal edges
                  for (auto const &internal_input : outputs)
                    {
                      for (auto &internal_output : graph.get_output_nodes(internal_input))
                        {
                          if (outputs.find(internal_output) != outputs.cend())
                            {
                              weight_cost +=
                                graph.get_weight(out_device_id, std::make_pair(internal_input, internal_output));
                            }
                        }
                    }
                }
              // (2+)-(2+). In this case, there are two possibilities: either the block graph mode is classic
              // (and the program should trow) or the block graph mode is input/output. In the latter case, we
              // should consider the transmission of the "frontier" nodes of inputs and the overall execution
              // cost for the outputs
              else
                {
                  // In classic mode, every edge can have at most one 2+ node.
                  if (mode == Block_Graph_Generation_Mode::classic)
                    {
                      throw std::logic_error("The edge (" + std::to_string(edge.first) + ", " +
                                             std::to_string(edge.second) + ") has both multiple inputs and outputs!");
                    }
                  // In input and output mode, every edge can have up to two 2+ nodes.
                  else
                    {
                      bool set = false;

                      // Compute the total weight associated to the internal edges
                      for (auto const &internal_input : outputs)
                        {
                          for (auto &internal_output : graph.get_output_nodes(internal_input))
                            {
                              if (outputs.find(internal_output) != outputs.cend())
                                {
                                  weight_cost +=
                                    graph.get_weight(out_device_id, std::make_pair(internal_input, internal_output));
                                  set = true;
                                }
                            }
                        }

                      if (!set)
                        {
                          std::stringstream stt;
                          stt << "Missing weight in block graph generation!" << std::endl;

                          throw std::logic_error(stt.str());
                        }
                    }
                }

              new_graph.set_weight(edge, weight_cost);
            }
        };

        std::for_each(nodes.cbegin(), nodes.cend(), add_weight_cost);
      }
  }


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::apply_transmission_weights(block_graph_type &new_graph)
  {
    using namespace network_butcher::parameters;

    auto const &nodes = new_graph.get_nodes();

    for (auto const &node : nodes)
      {
        auto const &inputs = *node.content.second;
        auto const  first  = node.get_id();

        auto const in_device_id = node.content.first;

        for (auto const &second : new_graph.get_output_nodes(first))
          {
            auto const &out_node = new_graph[second];

            edge_type const edge = {first, second};
            // The device id of the output node (=0 starting device, >0 other device)
            auto const out_device_id = out_node.content.first;

            // Look for the nodes of the original graph that are represented by the output node (in the
            // linearized graph)
            auto const &outputs = *out_node.content.second;

            double final_cost = 0.;

            // 1-1 correspondence
            if (outputs.size() == 1 && inputs.size() == 1)
              {
                auto const &input  = *inputs.begin();
                auto const &output = *outputs.begin();

                auto const tmp_edge = std::make_pair(input, output);

                final_cost = transmission_weights(input, in_device_id, out_device_id);
              }
            // (2+)-1 correspondence. The idea is that the input nodes must transmit to the output node the
            // different values. Thus, the transmission cost is paid several times.
            else if (outputs.size() == 1)
              {
                auto const &output = *outputs.begin();
                // The inputs on the original graph of the output node have to
                // transmit their values to the output node
                for (auto const &input : this->original_graph.get_input_nodes(output))
                  {
                    final_cost += transmission_weights(input, in_device_id, out_device_id);
                  }
              }
            // 1-(2+). In this case, the input is sent to the device of the output nodes a single time.
            // Thus, this transmission cost is taken into account only once.
            else if (inputs.size() == 1)
              {
                auto const &input        = *inputs.begin();
                auto const &comm_outputs = this->original_graph.get_output_nodes(input);

                final_cost += transmission_weights(input, in_device_id, out_device_id);
              }
            // (2+)-(2+). In this case, there are two possibilities: either the block graph mode is classic
            // (and the program should trow) or the block graph mode is input/output. In the latter case, we
            // should consider the transmission of the "frontier" nodes of inputs and the overall execution
            // cost for the outputs
            else
              {
                // In classic mode, every edge can have at most one 2+ node.
                if (params.block_graph_mode == Block_Graph_Generation_Mode::classic)
                  {
                    throw std::logic_error("The edge (" + std::to_string(edge.first) + ", " +
                                           std::to_string(edge.second) + ") has both multiple inputs and outputs!");
                  }
                // In input and output mode, every edge can have up to two 2+ nodes.
                else
                  {
                    // This is the collection of the input nodes of every node contained in outputs
                    std::set<node_id_type> output_node_inputs;

                    for (auto const &node_id : outputs)
                      {
                        auto const &tmp_nodes = this->original_graph.get_input_nodes(node_id);
                        output_node_inputs.insert(tmp_nodes.cbegin(), tmp_nodes.cend());
                      }

                    // This is the collection of nodes in inputs whose output tensors are fed to outputs
                    std::vector<node_id_type> frontier_input(std::max(inputs.size(), output_node_inputs.size()));
                    auto const                close_frontier = std::set_intersection(output_node_inputs.cbegin(),
                                                                      output_node_inputs.cend(),
                                                                      inputs.cbegin(),
                                                                      inputs.cend(),
                                                                      frontier_input.begin());

                    // We have to consider the transmission cost for every node in the frontier_input
                    for (auto input_it = frontier_input.cbegin(); input_it != close_frontier; ++input_it)
                      {
                        final_cost += transmission_weights(*input_it, in_device_id, out_device_id);
                      }
                  }
              }

            if (new_graph.check_weight(edge))
              final_cost += new_graph.get_weight(edge);

            new_graph.set_weight(edge, final_cost);
          }
      };
  }
} // namespace network_butcher

#endif // NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H
