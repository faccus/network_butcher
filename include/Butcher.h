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

#include "Path_Converter.h"

namespace network_butcher
{

  /// Butcher butchers a given graph into slices
  /// \tparam GraphType The type of the graph
  template <class GraphType>
  class Butcher
  {
  public:
    using network     = GraphType;
    using new_network = block_graph_type;

  private:
    using path_info = network_butcher::kfinder::path_info;

    network graph;


    /// It will produce a linearized version of the current graph (with multiple devices)
    /// \param backward_connections_allowed Allow backward connections between devices (i.e. data can be sent from
    /// device 2 to device 1) \param graph_in_device Input device id \param graph_out_device Output device id \return
    /// The linearized graph (with multiple devices) and the map that associated every node id of the original graph to
    /// the respective node id of the "new" graph
    [[nodiscard]] std::pair<new_network, std::map<node_id_type, node_id_type>>
    block_graph(bool                                                     backward_connections_allowed,
                std::size_t                                              graph_in_device,
                std::size_t                                              graph_out_device,
                network_butcher::parameters::Block_Graph_Generation_Mode mode =
                  network_butcher::parameters::Block_Graph_Generation_Mode::classic) const;


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


    /// Given the current graph and the original weight function, it will produce the weights for the linearized graph
    /// \param new_graph The linearized graph (result of block_graph)
    /// \param params The parameters
    /// \param transmission_weights They are used when we are switching from a device to another. The node is the source
    /// while the two size_t are the input and output device ids
    void
    block_graph_weights(
      new_network                                                                      &new_graph,
      network_butcher::parameters::Parameters const                                    &params,
      const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &transmission_weights) const;

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


    /// Computes the k shortest paths for the given graph
    /// \param transmission_weights The transmission weight function
    /// \param params The set of parameters
    /// \return The set of "best" paths
    network_butcher::types::Weighted_Real_Paths
    compute_k_shortest_path(
      std::function<weight_type(node_id_type const &, std::size_t, std::size_t)> const &transmission_weights,
      network_butcher::parameters::Parameters const                                    &params) const;
  };


  template <class GraphType>
  std::pair<typename Butcher<GraphType>::new_network, std::map<node_id_type, node_id_type>>
  Butcher<GraphType>::block_graph(bool                                                     backward_connections_allowed,
                                  std::size_t                                              graph_in_device,
                                  std::size_t                                              graph_out_device,
                                  network_butcher::parameters::Block_Graph_Generation_Mode mode) const
  {
    auto const linearize_graph = [](GraphType::Dependencies_Type const                             &old_dependencies,
                                    GraphType::Node_Collection_Type const                          &old_nodes,
                                    network_butcher::parameters::Block_Graph_Generation_Mode const &mode) {
      // Counter is used to establish if the current node has more output
      // connections than the inputs one.
      int counter = old_dependencies.front().second.size() - old_dependencies.front().first.size() - 1;

      std::list<new_network::Node_Type>    starting_nodes;
      std::map<node_id_type, node_id_type> old_to_new; // Old node -> New node


      starting_nodes.emplace_back(new_network::Node_Internal_Type{0, nullptr});
      starting_nodes.back().content.second = std::make_shared<node_id_collection_type>(node_id_collection_type{0});

      // Cycle through all the nodes of the graph
      for (auto it = ++old_nodes.begin(); it != old_nodes.end(); ++it)
        {
          // Node of the old graph
          auto const &node          = *it;
          auto const &dep           = old_dependencies[node.get_id()];
          int const   local_counter = dep.second.size() - dep.first.size();

          // Add new node
          if (local_counter <= 0 && counter == 0)
            {
              starting_nodes.emplace_back(new_network::Node_Internal_Type{0, nullptr});
              starting_nodes.back().content.second =
                std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});
            }
          // Add new node and add master node for next steps
          else if (local_counter > 0 && counter == 0)
            {
              starting_nodes.emplace_back(new_network::Node_Internal_Type{0, nullptr});
              starting_nodes.back().content.second =
                std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});

              starting_nodes.emplace_back(new_network::Node_Internal_Type{0, nullptr});
              starting_nodes.back().content.second = std::make_shared<node_id_collection_type>();

              counter += local_counter;
            }
          // Add node link to the "big" node
          else if ((local_counter == 0 && dep.second.size() == 1 || local_counter > 0 && dep.first.size() <= 1) &&
                   counter > 0)
            {
              starting_nodes.back().content.second->insert(starting_nodes.back().content.second->end(), node.get_id());

              counter += local_counter;
            }
          else if (counter > 0 && ((local_counter >= 0 && dep.first.size() > 1) || (local_counter < 0)))
            {
              counter -= (dep.first.size() - 1);

              // End of the master node
              if (counter == 0)
                {
                  starting_nodes.emplace_back(new_network::Node_Internal_Type{0, nullptr});
                  starting_nodes.back().content.second =
                    std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});

                  // Do we have to add another master node?
                  if (local_counter >= 0)
                    {
                      starting_nodes.emplace_back(new_network::Node_Internal_Type{0, nullptr});
                      starting_nodes.back().content.second = std::make_shared<node_id_collection_type>();
                    }
                }
              else
                {
                  starting_nodes.back().content.second->insert(starting_nodes.back().content.second->end(),
                                                               node.get_id());
                }

              counter += (dep.second.size() - 1);
            }
          else
            {
              throw std::runtime_error("Unknown node found during block_graph construction");
            }
        }

      if (starting_nodes.size() > 1)
        {
          if (mode == network_butcher::parameters::Block_Graph_Generation_Mode::input)
            {
              for (auto it = ++starting_nodes.begin(), it_follower = starting_nodes.begin(); it != starting_nodes.end();
                   ++it, ++it_follower)
                {
                  if (it->content.second->size() > 1)
                    {
                      auto &content_big_node = it->content.second;

                      it_follower->content.second->insert(std::make_move_iterator(content_big_node->begin()),
                                                          std::make_move_iterator(content_big_node->end()));

                      starting_nodes.erase(it);
                      it = ++it_follower;

                      if (it == starting_nodes.end())
                        break;
                    }
                }
            }
          else if (mode == network_butcher::parameters::Block_Graph_Generation_Mode::output)
            {
              for (auto it          = ++starting_nodes.begin(),
                        it_follower = starting_nodes.begin(),
                        it_mem      = starting_nodes.begin();
                   it != starting_nodes.end();
                   ++it, ++it_follower)
                {
                  if (it->content.second->size() > 1)
                    {
                      it_mem = it_follower;
                    }
                  else if (it_follower->content.second->size() > 1)
                    {
                      auto &content_small_node = it->content.second;

                      it_follower->content.second->insert(std::make_move_iterator(content_small_node->begin()),
                                                          std::make_move_iterator(content_small_node->end()));

                      starting_nodes.erase(it);
                      it          = it_follower;
                      it_follower = it_mem;
                    }
                }
            }
        }

      return std::tuple{starting_nodes, old_to_new};
    };

    auto const add_extra_nodes_per_device = [](std::list<new_network::Node_Type> &starting_nodes,
                                               std::size_t                        num_devices) {
      for (auto it_follower = ++starting_nodes.cbegin(), it = ++(++starting_nodes.cbegin());
           it != starting_nodes.cend();
           it_follower = it, ++it)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              starting_nodes.emplace(it, new_network::Node_Internal_Type{i, it_follower->content.second});
            }
        }
    };

    auto const process_dependencies =
      [backward_connections_allowed](std::size_t dep_size, std::size_t supp_size, std::size_t num_devices) {
        new_network::Dependencies_Type new_dependencies;
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

              if (!backward_connections_allowed)
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

              if (!backward_connections_allowed)
                in.insert(in.end(), id - num_devices);


              for (std::size_t k = 0; k < num_devices; ++k)
                {
                  if (backward_connections_allowed)
                    in.insert(in.end(), id - num_devices + k);

                  out.insert(out.end(), id + num_devices + k);
                }

              for (std::size_t k = 1; k < num_devices; ++k)
                {
                  auto tmp_dep = new_dependencies.back();

                  if (!backward_connections_allowed)
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

          if (backward_connections_allowed)
            for (std::size_t k = 1; k < num_devices; ++k)
              in.insert(in.end(), id - num_devices + k);

          for (std::size_t k = 1; k < num_devices; ++k)
            {
              auto dep_cpy = new_dependencies.back();

              if (!backward_connections_allowed)
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

        return std::move(new_dependencies);
      };

    auto [starting_nodes, old_to_new] = linearize_graph(graph.get_neighbors(), graph.get_nodes(), mode);

    auto const supp_size = starting_nodes.size() - 2;
    if (starting_nodes.size() > 2)
      add_extra_nodes_per_device(starting_nodes, graph.get_num_devices());

    new_network::Node_Collection_Type new_nodes;
    new_nodes.reserve(starting_nodes.size());

    new_nodes.insert(new_nodes.end(),
                     std::make_move_iterator(starting_nodes.begin()),
                     std::make_move_iterator(starting_nodes.end()));

    new_nodes.front().content.first = graph_in_device;
    new_nodes.back().content.first  = graph_out_device;

    return {new_network(new_nodes, process_dependencies(new_nodes.size(), supp_size, graph.get_num_devices())),
            old_to_new};
  }


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
  void
  Butcher<GraphType>::block_graph_weights(
    new_network                                                                      &new_graph,
    network_butcher::parameters::Parameters const                                    &params,
    const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &transmission_weights) const
  {
    using namespace network_butcher::parameters;

    auto const &nodes = new_graph.get_nodes();

    if (params.weight_import_mode == Weight_Import_Mode::aMLLibrary_block)
      {
        if constexpr (std::is_same_v<GraphType, graph_type>)
          {
            io::block_aMLLibrary_Weight_Importer(graph, new_graph, params).import_weights();
          }
        else
          {
            throw std::runtime_error("The specified weight import mode is not supported!");
          }
      }
    else
      {
        auto const add_weight_cost = [&new_graph,
                                      &graph = graph,
                                      &transmission_weights,
                                      mode = params.block_graph_mode](new_network::Node_Type const &node) {
          auto const first = node.get_id();

          // Look for the nodes of the original graph that are
          // represented by the input node (in the linearized
          // graph)
          auto const &inputs = *node.content.second;

          // The device id of the input node (=0 starting device, >0 other device)
          auto const in_device_id = node.content.first;

          for (auto const &second : new_graph.get_neighbors()[first].second)
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
                  auto const &inputs_of_output = graph.get_neighbors()[output].first;

                  weight_cost = graph.get_weight(out_device_id, std::make_pair(*inputs_of_output.cbegin(), output));
                }
              // 1-(2+) correspondence
              else if (inputs.size() == 1)
                {
                  auto const &input             = *inputs.begin();
                  auto const &interface_outputs = graph.get_neighbors()[input].second;

                  for (auto const &output : interface_outputs)
                    weight_cost += graph.get_weight(out_device_id, std::make_pair(input, output));

                  // Compute the total weight associated to the internal edges
                  for (auto const &internal_input : outputs)
                    {
                      for (auto &internal_output : graph.get_neighbors()[internal_input].second)
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
                          for (auto &internal_output : graph.get_neighbors()[internal_input].second)
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

    auto const add_transmission_weights = [&new_graph,
                                           &graph = graph,
                                           &transmission_weights,
                                           mode = params.block_graph_mode](new_network::Node_Type const &node) {
      auto const &inputs = *node.content.second;
      auto const  first  = node.get_id();

      auto const in_device_id = node.content.first;

      for (auto const &second : new_graph.get_neighbors()[first].second)
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
              for (auto const &input : graph.get_neighbors()[output].first)
                {
                  final_cost += transmission_weights(input, in_device_id, out_device_id);
                }
            }
          // 1-(2+). In this case, the input is sent to the device of the output nodes a single time.
          // Thus, this transmission cost is taken into account only once.
          else if (inputs.size() == 1)
            {
              auto const &input        = *inputs.begin();
              auto const &comm_outputs = graph.get_neighbors()[input].second;

              final_cost += transmission_weights(input, in_device_id, out_device_id);
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
                  // This is the collection of the input nodes of every node contained in outputs
                  std::set<node_id_type> output_node_inputs;

                  for (auto const &node_id : outputs)
                    {
                      auto const &nodes = graph.get_neighbors()[node_id].first;
                      output_node_inputs.insert(nodes.cbegin(), nodes.cend());
                    }

                  // This is the collection of nodes in inputs whose output tensors are fed to outputs
                  std::vector<node_id_type> frontier_input;
                  frontier_input.reserve(std::max(inputs.size(), output_node_inputs.size()));
                  std::set_intersection(output_node_inputs.cbegin(),
                                        output_node_inputs.cend(),
                                        inputs.cbegin(),
                                        inputs.cend(),
                                        frontier_input.begin());

                  // We have to consider the transmission cost for every node in the frontier_input
                  for (auto const &input : frontier_input)
                    {
                      final_cost += transmission_weights(input, in_device_id, out_device_id);
                    }
                }
            }

          if (new_graph.check_weight(edge))
            final_cost += new_graph.get_weight(edge);

          new_graph.set_weight(edge, final_cost);
        }
    };

    std::for_each(nodes.cbegin(), nodes.cend(), add_transmission_weights);
  }


  template <class GraphType>
  network_butcher::types::Weighted_Real_Paths
  Butcher<GraphType>::compute_k_shortest_path(
    const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &transmission_weights,
    const network_butcher::parameters::Parameters                                    &params) const
  {
    using namespace network_butcher::kfinder;

    auto [new_graph, connection_map] =
      block_graph(params.backward_connections_allowed, params.starting_device_id, params.ending_device_id);

    if (params.memory_constraint != network_butcher::parameters::Memory_Constraint_Type::None &&
        !params.backward_connections_allowed)
      remove_unfeasible_paths(params.devices, new_graph, params.memory_constraint_type);

    block_graph_weights(new_graph, params, transmission_weights);

    auto kFinder = KFinder_Factory<new_network>::Instance().create(params.method, new_graph);

    auto const res = kFinder->compute(params.K);

    network_butcher::Utilities::Path_Converter converter(new_graph);
    return converter.convert_to_weighted_real_path(res);
  }

} // namespace network_butcher
#endif // NETWORK_BUTCHER_BUTCHER_H