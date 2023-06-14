#ifndef NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H
#define NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H

#include <list>

#include <network_butcher/Butcher/graph_constraint.h>
#include <network_butcher/IO_Interaction/weight_importers.h>
#include <network_butcher/Network/graph_traits.h>

namespace network_butcher
{
  /// This class will be used to generate a block graph from a graph, applying the provided constraints. It will take
  /// the input graph and the parameters as constant reference and, based on the graph structure, it will generate the
  /// block graph. The constraints are applied after the block graph is generated. Weight importers can be used to
  /// import weights from external sources directly in the block graph.
  /// \tparam GraphType The original graph type
  template <typename GraphType>
  class Constrained_Block_Graph_Builder
  {
  protected:
    /// Alias for a transmission function
    using transmission_func_type = std::function<Time_Type(const Edge_Type &, std::size_t, std::size_t)>;

    /// Original graph. Used to create the block graph
    GraphType const &original_graph;

    /// The collection of constraints
    std::vector<std::unique_ptr<constraints::Graph_Constraint>> constraints;

    /// The collection of block graph related parameters
    parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params;

    /// The collection of aMLLibrary related parameters
    parameters::Parameters::aMLLibrary const &aMLLibrary_params;

    /// The collection of weights related parameters
    parameters::Parameters::Weights const &weights_params;

    /// The collection of model related parameters
    parameters::Parameters::Model const &model_params;

    /// The collection of devices
    parameters::Parameters::Devices const &devices;

    /// Internal flag. Set to true if operation weights should be used
    bool weights;

    /// Internal transmission function. Set to nullptr if transmission weights should not be used
    transmission_func_type transmission_weights;


    /// The actual construction of the block graph is performed when this function is called
    /// \return The block graph
    [[nodiscard]] auto
    build_block_graph() const -> Block_Graph_Type;

    /// Apply to the input block graph the operation weights from the original graph
    /// \param new_graph The block graph
    void
    apply_operation_weights(Block_Graph_Type &new_graph) const;

    /// Apply to the input block graph the transmission weights
    /// \param new_graph The block graph
    void
    apply_transmission_weights(Block_Graph_Type &new_graph) const;

    /// Apply to the input graph the given collection of constraints
    /// \param new_graph The block graph
    void
    apply_constraints(Block_Graph_Type &new_graph) const;

    /// Try to import operation weights using an importer
    /// \param new_graph The block graph
    /// \return True if the import was successful, false if it was not performed
    auto
    apply_weights_from_importer(Block_Graph_Type &new_graph) const -> bool;


  public:
    /// Simple constructor for a Constrained Block Graph Builder
    /// \param original_graph The original graph. This is saved through a const reference
    /// \param block_graph_generation_params Block Graph Generation parameters
    /// \param aMLLibrary_params aMLLibrary parameters
    /// \param weights_params Weights parameters
    /// \param model_params Model parameters
    /// \param devices The collection of devices
    /// \param initial_constraint_gen A function that should generate the initial constraints for the builder
    explicit Constrained_Block_Graph_Builder(
      GraphType const                                                                    &original_graph,
      parameters::Parameters::Block_Graph_Generation const                               &block_graph_generation_params,
      parameters::Parameters::aMLLibrary const                                           &aMLLibrary_params,
      parameters::Parameters::Weights const                                              &weights_params,
      parameters::Parameters::Model const                                                &model_params,
      parameters::Parameters::Devices const                                              &devices,
      std::function<std::vector<std::unique_ptr<constraints::Graph_Constraint>>()> const &initial_constraint_gen =
        nullptr)
      : original_graph(original_graph)
      , block_graph_generation_params{block_graph_generation_params}
      , aMLLibrary_params{aMLLibrary_params}
      , weights_params{weights_params}
      , model_params{model_params}
      , devices{devices}
      , weights{false}
      , transmission_weights(nullptr)
    {
      if (initial_constraint_gen)
        {
          constraints = initial_constraint_gen();
        }
    };

    /// Simple constructor for a Constrained Block Graph Builder
    /// \param original_graph The original graph. This is saved through a const reference
    /// \param params Parameters of the program
    /// \param initial_constraint_gen A function that should generate the initial constraints for the builder
    explicit Constrained_Block_Graph_Builder(
      GraphType const                                                                    &original_graph,
      parameters::Parameters const                                                       &params,
      std::function<std::vector<std::unique_ptr<constraints::Graph_Constraint>>()> const &initial_constraint_gen =
        nullptr)
      : original_graph(original_graph)
      , block_graph_generation_params{params.block_graph_generation_params}
      , aMLLibrary_params{params.aMLLibrary_params}
      , weights_params{params.weights_params}
      , model_params{params.model_params}
      , devices{params.devices}
      , weights{false}
      , transmission_weights(nullptr)
    {
      if (initial_constraint_gen)
        {
          constraints = initial_constraint_gen();
        }
    };


    /// Call this function if the builder should apply the transmission weights during the block graph construction
    /// \param in_transmission_weights The transmission weights
    void
    construct_transmission_weights(transmission_func_type const &in_transmission_weights);


    /// Call this function if the builder should apply the weights from the original graph during the block graph
    /// construction
    void
    construct_operation_weights();


    /// Call this function if the builder should apply the transmission weights and the weights from the original graph
    /// during the block graph construction
    /// \param in_transmission_weights The transmission weights
    void
    construct_weights(transmission_func_type const &in_transmission_weights);


    /// Add to the collection of constraints the input constraint
    /// \param constraint A r-value reference to the new constraint
    void
    add_constraint(std::unique_ptr<constraints::Graph_Constraint> &&constraint);


    /// The basic construct method. It will produce the block graph using the specified options.
    /// \return The resulting block graph
    [[nodiscard]] auto
    construct_block_graph() const -> Block_Graph_Type;


    ~Constrained_Block_Graph_Builder() = default;
  };


  template <typename GraphType>
  auto
  Constrained_Block_Graph_Builder<GraphType>::apply_weights_from_importer(Block_Graph_Type &new_graph) const -> bool
  {
    using namespace network_butcher::parameters;

    // Check if we have to generate the weights though aMLLibrary
    if (weights_params.weight_import_mode == Weight_Import_Mode::aMLLibrary_block)
      {
        // aMLLibrary can only be applied if we are dealing with an Onnx model.
        if constexpr (std::is_same_v<GraphType, Converted_Onnx_Graph_Type>)
          {
            io::block_aMLLibrary_Weight_Importer(this->original_graph,
                                                 new_graph,
                                                 block_graph_generation_params,
                                                 aMLLibrary_params,
                                                 weights_params,
                                                 model_params,
                                                 devices)
              .import_weights();
          }
        else
          {
            throw std::runtime_error("Constrained_Block_Graph_Builder: The specified weight import mode is not "
                                     "supported or not compatible with the chosen graph type!");
          }
      }
    // Check if we have to import the block graph weights directly from a .csv file
    else if (weights_params.weight_import_mode == Weight_Import_Mode::block_single_direct_read)
      {
        io::Csv_Weight_Importer<Block_Graph_Type>(new_graph,
                                                  {weights_params.single_weight_import_path},
                                                  weights_params.single_csv_columns_weights,
                                                  devices,
                                                  weights_params.separator)
          .import_weights();
      }
    // Check if we have to import the block graph weights directly from multiple .csv files
    else if (weights_params.weight_import_mode == Weight_Import_Mode::block_multiple_direct_read)
      {
        io::Csv_Weight_Importer<Block_Graph_Type>(new_graph, devices, weights_params.separator).import_weights();
      }
    else
      return false;

    return true;
  }


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::construct_weights(const transmission_func_type &in_transmission_weights)
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
    const transmission_func_type &in_transmission_weights)
  {
    this->transmission_weights = in_transmission_weights;
  }


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::add_constraint(
    std::unique_ptr<constraints::Graph_Constraint> &&constraint)
  {
    constraints.emplace_back(std::move(constraint));
  };


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::apply_constraints(Block_Graph_Type &new_graph) const
  {
    for (auto const &constraint : constraints)
      {
        constraint->apply_constraint(new_graph);
      }
  };


  template <typename GraphType>
  auto
  Constrained_Block_Graph_Builder<GraphType>::build_block_graph() const -> Block_Graph_Type
  {
    // If the original graph has one node, the block graph will have a single node
    if (original_graph.size() == 1)
      {
        std::vector<Block_Graph_Type::Node_Type> res;
        res.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
        res.back().content.second = std::make_shared<Node_Id_Collection_Type>(Node_Id_Collection_Type{0});

        return Block_Graph_Type(std::move(res), Block_Graph_Type::Neighbours_Type(1));
      }
    // If the original graph has two nodes, the block graph wil be made by two nodes
    else if (original_graph.size() == 2)
      {
        std::vector<Block_Graph_Type::Node_Type> res;

        res.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
        res.back().content.second = std::make_shared<Node_Id_Collection_Type>(Node_Id_Collection_Type{0});

        res.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
        res.back().content.second = std::make_shared<Node_Id_Collection_Type>(Node_Id_Collection_Type{1});

        Block_Graph_Type::Neighbours_Type deps(2);
        deps.front().second.insert(1);
        deps.back().first.insert(0);

        return Block_Graph_Type(std::move(res), std::move(deps));
      }


    // It will construct the linearized version of the original graph
    auto const linearize_graph = [](GraphType const                                                &old_graph,
                                    network_butcher::parameters::Block_Graph_Generation_Mode const &mode) {
      auto const &old_nodes = old_graph.get_nodes();

      // Counter is used to establish if the current node has more output
      // connections than the inputs one.
      int counter = old_graph.get_output_nodes(0).size() - old_graph.get_input_nodes(0).size() - 1;

      std::list<Block_Graph_Type::Node_Type> starting_nodes;

      // If counter is immediately greater than one, the model input is used by two or more layers. Thus, we will
      // immediately insert a block node. The input padding node will insert later
      if (counter > 0)
        {
          starting_nodes.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
          starting_nodes.back().content.second = std::make_shared<Node_Id_Collection_Type>();
        }

      // Cycle through all the nodes of the graph
      for (auto it = ++old_nodes.cbegin(); it != (++old_nodes.crbegin()).base(); ++it)
        {
          // Node of the old graph
          auto const &node        = *it;
          auto const &input_deps  = old_graph.get_input_nodes(node.get_id());
          auto const &output_deps = old_graph.get_output_nodes(node.get_id());

          int const local_counter = output_deps.size() - input_deps.size();

          // Add new node
          if (local_counter <= 0 && counter == 0)
            {
              starting_nodes.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
              starting_nodes.back().content.second =
                std::make_shared<Node_Id_Collection_Type>(Node_Id_Collection_Type{node.get_id()});
            }
          // Add new node and add block node for next steps
          else if (local_counter > 0 && counter == 0)
            {
              starting_nodes.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
              starting_nodes.back().content.second =
                std::make_shared<Node_Id_Collection_Type>(Node_Id_Collection_Type{node.get_id()});

              starting_nodes.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
              starting_nodes.back().content.second = std::make_shared<Node_Id_Collection_Type>();

              counter += local_counter;
            }
          // Add node link to the block node
          else if ((local_counter == 0 && output_deps.size() == 1 || local_counter > 0 && input_deps.size() <= 1) &&
                   counter > 0)
            {
              starting_nodes.back().content.second->insert(starting_nodes.back().content.second->end(), node.get_id());

              counter += local_counter;
            }
          else if (counter > 0 && ((local_counter >= 0 && input_deps.size() > 1) || (local_counter < 0)))
            {
              counter -= (input_deps.size() - 1);

              // End of the block node
              if (counter == 0)
                {
                  starting_nodes.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
                  starting_nodes.back().content.second =
                    std::make_shared<Node_Id_Collection_Type>(Node_Id_Collection_Type{node.get_id()});

                  // Do we have to add another block node?
                  if (local_counter >= 0)
                    {
                      starting_nodes.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
                      starting_nodes.back().content.second = std::make_shared<Node_Id_Collection_Type>();
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
              throw std::runtime_error(
                "Constrained_Block_Graph_Builder: Unknown node found during block_graph construction");
            }
        }

      // Add front node
      starting_nodes.emplace_front(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
      starting_nodes.front().content.second = std::make_shared<Node_Id_Collection_Type>(Node_Id_Collection_Type{0});

      // Add back node
      starting_nodes.emplace_back(Block_Graph_Type::Node_Type::Content_Type{0, nullptr});
      starting_nodes.back().content.second =
        std::make_shared<Node_Id_Collection_Type>(Node_Id_Collection_Type{old_nodes.crbegin()->get_id()});


      // If the block graph mode is not set to classic, then we need to merge either the input or the output nodes of
      // the block nodes
      if (starting_nodes.size() > 3 && mode != network_butcher::parameters::Block_Graph_Generation_Mode::classic)
        {
          // Simple lambda to be used to merge the nodes contained in the two specified collections. It will also change
          // the two original iterators. Returns false if the new iterator is the container_end
          auto const merge_nodes = [&starting_nodes](auto &it_succ, auto &it_prec, auto const &container_end) {
            auto &it_nodes_edit = it_succ->content.second;

            it_prec->content.second->insert(std::make_move_iterator(it_nodes_edit->begin()),
                                            std::make_move_iterator(it_nodes_edit->end()));

            starting_nodes.erase(it_succ);
            it_succ = it_prec;
            ++it_succ;

            if (it_succ == container_end)
              return true;

            return false;
          };

          if (mode == network_butcher::parameters::Block_Graph_Generation_Mode::input)
            {
              // Loops through the starting nodes looking for a block node
              for (auto it_succ = ++(++starting_nodes.begin()), it_prec = ++starting_nodes.begin();
                   it_succ != starting_nodes.end();
                   ++it_succ, ++it_prec)
                {
                  auto const &it_prec_nodes_const = it_prec->content.second;

                  // If we detect that it_prec points to the input of a block node....
                  if (it_prec_nodes_const->size() == 1 &&
                      old_graph.get_output_nodes(*it_prec_nodes_const->cbegin()).size() > 1)
                    {
                      // It will merge them
                      if (merge_nodes(it_succ, it_prec, starting_nodes.end()))
                        break;
                    }
                }
            }
          else if (mode == network_butcher::parameters::Block_Graph_Generation_Mode::output)
            {
              // Loops through the starting nodes looking for a block node
              for (auto it_succ = ++starting_nodes.begin(), it_prec = starting_nodes.begin();
                   it_succ != (++starting_nodes.crbegin()).base();
                   ++it_succ, ++it_prec)
                {
                  auto const &it_nodes_const = it_succ->content.second;

                  // If we detect that it_succ points to the output of a block node....
                  if (it_nodes_const->size() == 1 && old_graph.get_input_nodes(*it_nodes_const->cbegin()).size() > 1)
                    {
                      // It will merge them
                      if (merge_nodes(it_succ, it_prec, (++starting_nodes.crbegin()).base()))
                        break;
                    }
                }
            }
        }

      return starting_nodes;
    };

    // It will add all the nodes required in the block graph
    auto const add_extra_nodes_per_device = [](std::list<Block_Graph_Type::Node_Type> &starting_nodes,
                                               std::size_t                             num_devices) {
      if (starting_nodes.size() > 2)
        {
          for (auto it_follower = ++starting_nodes.cbegin(), it = ++(++starting_nodes.cbegin());
               it != starting_nodes.cend();
               it_follower = it, ++it)
            {
              for (std::size_t i = 1; i < num_devices; ++i)
                {
                  starting_nodes.emplace(it, Block_Graph_Type::Node_Type::Content_Type{i, it_follower->content.second});
                }
            }
        }
    };

    // It will produce the dependencies for the block graph
    auto const process_full_dependencies = [](std::size_t dep_size, std::size_t supp_size, std::size_t num_devices) {
      Block_Graph_Type::Neighbours_Type new_dependencies;
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
        new_dependencies.emplace_back(std::make_pair<Node_Id_Collection_Type, Node_Id_Collection_Type>({0}, {}));
        auto &out = new_dependencies.back().second;
        for (std::size_t k = 0; k < num_devices; ++k)
          out.insert(out.end(), 1 + num_devices + k);

        for (std::size_t k = 1; k < num_devices; ++k)
          {
            new_dependencies.emplace_back(new_dependencies.back());
          }
      }

      // Inputs: previous layer nodes, Outputs: following layer nodes
      if (supp_size > 1)
        {
          new_dependencies.resize(new_dependencies.size() + (supp_size - 1) * num_devices);

          auto const func = [&new_dependencies, &num_devices](auto const &i) {
            auto const id = num_devices * i + 1;

            auto &[in, out] = new_dependencies[id];

            for (std::size_t k = 0; k < num_devices; ++k)
              {
                in.insert(in.end(), id - num_devices + k);
              }

            for (std::size_t k = 0; k < num_devices; ++k)
              {
                out.insert(out.end(), id + num_devices + k);
              }

            for (std::size_t k = 1; k < num_devices; ++k)
              {
                new_dependencies[id + k].first  = in;
                new_dependencies[id + k].second = out;
              }
          };

#if NETWORK_BUTCHER_PARALLEL_TBB
          // Here, we would like to use the C++ (native) integration with TBB. So... we need to create a vector of
          // indices to be used by the parallel for_each. We will use the indices to access the new_dependencies

          // In C++26, in theory, we will be able to use std::views and std::ranges::for_each with parallel policies
          // to speed up the process
          std::vector<std::size_t> v(supp_size - 1);
          std::generate(v.begin(), v.end(), [n = 1]() mutable { return n++; });

          std::for_each(std::execution::par, v.cbegin(), v.cend(), func);
#else
#  pragma omp parallel default(none) shared(supp_size, func)
          {
#  pragma omp for
            for (std::size_t i = 1; i < supp_size; ++i)
              {
                func(i);
              }
          }
#endif
        }

      {
        // Inputs: previous layer nodes, Outputs: last node
        auto const id = new_dependencies.size() - num_devices;

        for (std::size_t k = 0; k < num_devices; ++k)
          {
            new_dependencies[id + k].second.clear();
            new_dependencies[id + k].second.insert(id + num_devices);
          }
      }

      // The last layer is fully connected with the last node
      {
        new_dependencies.emplace_back();

        auto &in = new_dependencies.back().first;
        for (std::size_t k = 0; k < num_devices; ++k)
          in.insert(in.end(), new_dependencies.size() - 1 - num_devices + k);
      }

      return new_dependencies;
    };

    // It will produce the dependencies for the block graph
    auto const process_partial_dependencies =
      [&weights_params = weights_params,
       &block_graph_generation_params =
         block_graph_generation_params](std::size_t dep_size, std::size_t supp_size, std::size_t num_devices) {
        Block_Graph_Type::Neighbours_Type new_dependencies;
        new_dependencies.reserve(dep_size);

        auto const &bandwidth = weights_params.bandwidth;

        // Node 0, Input: -, Outputs: following layer nodes
        {
          new_dependencies.emplace_back();
          auto &out = new_dependencies.back().second;

          for (auto const &neighbour : bandwidth->get_output_nodes(block_graph_generation_params.starting_device_id))
            {
              out.insert(out.end(), 1 + neighbour);
            }

          if (out.size() != num_devices)
            {
              for (Node_Id_Type i = 0; i < num_devices; ++i)
                {
                  // Check if it is in the neighbour or if it's allowed by an input bandwidth
                  if (!out.contains(i + 1) &&
                      weights_params.in_bandwidth.find(std::pair(0, i)) != weights_params.in_bandwidth.cend())
                    {
                      out.insert(out.end(), 1 + i);
                    }
                }
            }
        }

        // Node 1...num_devices , Inputs: first node, Outputs: following layer nodes
        {
          auto const &root_node_outs = new_dependencies.back().second;
          for (Node_Id_Type k = 0; k < num_devices; ++k)
            {
              new_dependencies.emplace_back();

              if (root_node_outs.contains(k + 1))
                {
                  new_dependencies.back().first.insert(new_dependencies.back().first.end(), 0);
                }

              for (auto const &neighbour : bandwidth->get_output_nodes(k))
                {
                  new_dependencies.back().second.insert(new_dependencies.back().second.end(),
                                                        1 + num_devices + neighbour);
                }
            }
        }

        // Nodes up to final_size - 1 - num_devices, Inputs: previous layer nodes, Outputs: following layer nodes
        if (supp_size > 1)
          {
            new_dependencies.resize(new_dependencies.size() + (supp_size - 1) * num_devices);

            auto const func = [&num_devices, &bandwidth, &new_dependencies](auto const &i) {
              auto const base_id = num_devices * i + 1;
              for (Device_Id_Type k = 0; k < num_devices; ++k)
                {
                  auto &[in, out] = new_dependencies[base_id + k];

                  for (auto const &neighbour : bandwidth->get_input_nodes(k))
                    {
                      in.insert(in.end(), base_id - num_devices + neighbour);
                    }

                  for (auto const &neighbour : bandwidth->get_output_nodes(k))
                    {
                      out.insert(out.end(), base_id + num_devices + neighbour);
                    }
                }
            };

#if NETWORK_BUTCHER_PARALLEL_TBB
            std::vector<Node_Id_Type> v(supp_size - 1);
            std::generate(v.begin(), v.end(), [n = 1]() mutable { return n++; });

            std::for_each(std::execution::par, v.cbegin(), v.cend(), func);
#else
#  pragma omp parallel default(none) shared(supp_size, func)
            {
#  pragma omp for
              for (Node_Id_Type i = 1; i < supp_size; ++i)
                {
                  func(i);
                }
            }
#endif
          }

        // Nodes final_size - 1 - num_devices, ..., final_size - 1 - 1, Inputs: previous layer nodes, Outputs: last node
        {
          auto const  base_id            = new_dependencies.size() - num_devices;
          auto const &device_inputs_sink = bandwidth->get_input_nodes(block_graph_generation_params.ending_device_id);

          for (Device_Id_Type k = 0; k < num_devices; ++k)
            {
              auto &out = new_dependencies[base_id + k].second;
              out.clear();

              if (device_inputs_sink.contains(k) ||
                  weights_params.out_bandwidth.find(std::pair(k, block_graph_generation_params.ending_device_id)) !=
                    weights_params.out_bandwidth.cend())
                {
                  out.insert(out.end(), base_id + num_devices);
                }
            }
        }

        // Node final_size-1, Inputs: previous layer nodes, Output: -
        {
          auto const  base_id            = new_dependencies.size();
          auto const &device_inputs_sink = bandwidth->get_input_nodes(block_graph_generation_params.ending_device_id);
          new_dependencies.emplace_back();

          auto &in = new_dependencies.back().first;
          for (Device_Id_Type k = 0; k < num_devices; ++k)
            {
              // Check if it is in the neighbor or if it's allowed by an output bandwidth
              if (device_inputs_sink.contains(k) ||
                  weights_params.out_bandwidth.find(std::pair(k, block_graph_generation_params.ending_device_id)) !=
                    weights_params.out_bandwidth.cend())
                {
                  in.insert(in.end(), base_id - num_devices + k);
                }
            }
        }

        return new_dependencies;
      };

    // Prepare the collection of nodes
    Block_Graph_Type::Node_Collection_Type new_nodes;
    std::size_t                            supp_size = 0;

    {
      // Get the linearized graph
      auto starting_nodes = linearize_graph(this->original_graph, block_graph_generation_params.block_graph_mode);
      supp_size           = starting_nodes.size() - 2;

      // Add the required nodes to the collection of nodes
      add_extra_nodes_per_device(starting_nodes, this->original_graph.get_num_devices());

      new_nodes.reserve(starting_nodes.size());

      new_nodes.insert(new_nodes.end(),
                       std::make_move_iterator(starting_nodes.begin()),
                       std::make_move_iterator(starting_nodes.end()));
    }

    // Fix the input/output device ids
    new_nodes.front().content.first = block_graph_generation_params.starting_device_id;
    new_nodes.back().content.first  = block_graph_generation_params.ending_device_id;

    auto const node_size = new_nodes.size();

    if (block_graph_generation_params.use_bandwidth_to_manage_connections)
      {
        return Block_Graph_Type(std::move(new_nodes),
                                process_partial_dependencies(node_size,
                                                             supp_size,
                                                             this->original_graph.get_num_devices()));
      }
    else
      {
        return Block_Graph_Type(std::move(new_nodes),
                                process_full_dependencies(node_size,
                                                          supp_size,
                                                          this->original_graph.get_num_devices()));
      }
  }


  template <typename GraphType>
  auto
  Constrained_Block_Graph_Builder<GraphType>::construct_block_graph() const -> Block_Graph_Type
  {
    // Construct the unweighted block graph
    auto new_graph = build_block_graph();

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
  Constrained_Block_Graph_Builder<GraphType>::apply_operation_weights(Block_Graph_Type &new_graph) const
  {
    using namespace network_butcher::parameters;

    // If we can import the weight with an importer, we return. Otherwise, we read the weights from the original graph
    if (apply_weights_from_importer(new_graph))
      return;

    auto const &nodes = new_graph.get_nodes();
    auto const &graph = this->original_graph;
    auto const &mode  = block_graph_generation_params.block_graph_mode;

    auto const process_node = [&new_graph, &graph, &mode](auto const &node) {
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

          Edge_Type const edge = std::make_pair(first, second);
          // The device id of the output node (=0 starting device, >0 other device)
          auto const out_device_id = out_node.content.first;

          // Look for the nodes of the original graph that are represented by the output node (in the
          // linearized graph)
          auto const &outputs = *out_node.content.second;

          Time_Type weight_cost = 0.;

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
                      // Is the internal_output part of the block node?
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
              // In classic mode, (2+)-(2+) edges are not allowed! There is an error somewhere
              if (mode == Block_Graph_Generation_Mode::classic)
                {
                  throw std::logic_error("Constrained_Block_Graph_Builder::apply_operation_weights: The edge (" +
                                         std::to_string(edge.first) + ", " + std::to_string(edge.second) +
                                         ") has both multiple inputs and outputs!");
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
                      throw std::logic_error("Constrained_Block_Graph_Builder::apply_operation_weights: Missing weight "
                                             "in block graph generation!");
                    }
                }
            }

          new_graph.set_weight(edge, weight_cost);
        }
    };

#if NETWORK_BUTCHER_PARALLEL_TBB
    std::for_each(nodes.cbegin(), nodes.cend(), process_node);
#else
#  pragma omp parallel default(none) shared(nodes, new_graph, graph, mode, process_node)
    {
#  pragma omp for
      for (const auto &node : nodes)
        {
          process_node(node);
        }
    }
#endif
  }


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::apply_transmission_weights(Block_Graph_Type &new_graph) const
  {
    using namespace network_butcher::parameters;

    auto const &nodes      = new_graph.get_nodes();
    auto const &graph      = this->original_graph;
    auto const &mode       = block_graph_generation_params.block_graph_mode;
    auto const &ts_weights = transmission_weights;

    auto const process_node = [&new_graph, &graph, &mode, &ts_weights](auto const &node) {
      auto const &inputs = *node.content.second;
      auto const  first  = node.get_id();

      auto const in_device_id = node.content.first;

      for (auto const &second : new_graph.get_output_nodes(first))
        {
          auto const &out_node = new_graph[second];

          Edge_Type const edge = std::make_pair(first, second);
          // The device id of the output node (=0 starting device, >0 other device)
          auto const out_device_id = out_node.content.first;

          // Look for the nodes of the original graph that are represented by the output node (in the
          // linearized graph)
          auto const &outputs = *out_node.content.second;

          Time_Type final_cost = 0.;

          // 1-1 correspondence
          if (outputs.size() == 1 && inputs.size() == 1)
            {
              auto const &input  = *inputs.begin();
              auto const &output = *outputs.begin();

              auto const tmp_edge = std::make_pair(input, output);

              final_cost = ts_weights(tmp_edge, in_device_id, out_device_id);
            }
          // (2+)-1 correspondence. The idea is that the input nodes must transmit to the output node the
          // different values. Thus, the transmission cost is paid several times.
          else if (outputs.size() == 1)
            {
              auto const &output = *outputs.begin();
              // The inputs on the original graph of the output node have to
              // transmit their values to the output node
              for (auto const &input : graph.get_input_nodes(output))
                {
                  final_cost += ts_weights(std::make_pair(input, output), in_device_id, out_device_id);
                }
            }
          // 1-(2+). In this case, the input is sent to the device of the output nodes a single time.
          // Thus, this transmission cost is taken into account only once.
          else if (inputs.size() == 1)
            {
              auto const &input        = *inputs.begin();
              auto const &comm_outputs = graph.get_output_nodes(input);

              final_cost += ts_weights(std::make_pair(input, *comm_outputs.crbegin()), in_device_id, out_device_id);
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
                  throw std::logic_error("Constrained_Block_Graph_Builder::apply_transmission_weights: The edge (" +
                                         std::to_string(edge.first) + ", " + std::to_string(edge.second) +
                                         ") has both multiple inputs and outputs!");
                }
              // In input and output mode, every edge can have up to two 2+ nodes.
              else
                {
                  // This is the collection of the input nodes of every node contained in outputs
                  std::set<Node_Id_Type> output_node_inputs;
                  for (auto const &node_id : outputs)
                    {
                      auto const &tmp_nodes = graph.get_input_nodes(node_id);
                      output_node_inputs.insert(tmp_nodes.cbegin(), tmp_nodes.cend());
                    }

                  // This is the collection of nodes in inputs whose output tensors are fed to outputs
                  std::vector<Node_Id_Type> frontier_input(std::max(inputs.size(), output_node_inputs.size()));
                  auto const                close_frontier = std::set_intersection(output_node_inputs.cbegin(),
                                                                    output_node_inputs.cend(),
                                                                    inputs.cbegin(),
                                                                    inputs.cend(),
                                                                    frontier_input.begin());

                  // We have to consider the transmission cost for every node in the frontier_input
                  for (auto input_it = frontier_input.cbegin(); input_it != close_frontier; ++input_it)
                    {
                      final_cost += ts_weights(std::make_pair(*input_it, *graph.get_output_nodes(*input_it).crbegin()),
                                               in_device_id,
                                               out_device_id);
                    }
                }
            }

          if (new_graph.check_weight(edge))
            final_cost += new_graph.get_weight(edge);

          new_graph.set_weight(edge, final_cost);
        }
    };

#if NETWORK_BUTCHER_PARALLEL_TBB
    std::for_each(std::execution::par, nodes.cbegin(), nodes.cend(), process_node);
#else
#  pragma omp parallel default(none) shared(nodes, graph, new_graph, transmission_weights, mode, process_node)
    {
#  pragma omp for
      for (const auto &node : nodes)
        {
          process_node(node);
        }
    }
#endif
  }
} // namespace network_butcher

#endif // NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H
