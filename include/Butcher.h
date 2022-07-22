//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_BUTCHER_H
#define NETWORK_BUTCHER_BUTCHER_H

#include <algorithm>
#include <forward_list>

#include "Helpers/Traits/Graph_traits.h"
#include "Helpers/Traits/Hardware_traits.h"

#include "Helpers/Computer/Computer_memory.h"

#include "Network/Graph.h"
#include "Network/Node.h"

#include "Helpers/K-shortest_path/KEppstein.h"
#include "Helpers/K-shortest_path/KEppstein_lazy.h"

#include "Helpers/Types/Type_info.h"

#include "Helpers/Parameters.h"
#include "Helpers/Types/Paths.h"
#include "Helpers/Utilities.h"

/// Butcher butchers a given graph into slices
template <class GraphType>
class Butcher
{
private:
  using network     = GraphType;
  using new_network = WGraph<node_id_collection_type>;

  network graph;

  /// Compute the minimal connected graphs (with dependencies) containing every
  /// node
  /// \return returns the smallest connected sub-graph (with dependencies)
  /// that connects the first node with the n-th node
  [[nodiscard]] std::vector<Slice_Type>
  compute_basic_routes() const
  {
    std::vector<Slice_Type> basic_routes;
    basic_routes.reserve(graph.get_nodes().size());

    {
      Slice_Type tmp;
      tmp.insert(0);
      basic_routes.push_back(tmp);
    }

    for (int i = 1; i < graph.get_nodes().size(); ++i)
      {
        Slice_Type partial_res;
        partial_res.insert(i);

        const auto &input_nodes = graph.get_dependencies()[i].first;

        // Construct the current basic_route the i (just add {i} to the basic
        // routes of the parents).
        for (auto &node : input_nodes)
          partial_res.insert(basic_routes[node].begin(),
                             basic_routes[node].end());
        basic_routes.push_back(partial_res);
      }

    return basic_routes;
  };

  /// Given a vector of Real_Partition, verify which ones applied to tester
  /// return true. \param partitions The vector of input Real_Partition \param
  /// tester The tester function \return True if the tester function returns
  /// true for every Real_Partition
  static bool
  partition_checker(Real_Path const                                   &path,
                    const std::function<bool(const Real_Partition &)> &tester)
  {
    std::vector<Slice_Type> res;

    for (auto const &partition : path)
      if (!tester(partition))
        return false;

    return true;
  };


  /// It will produce a linearized version of the current graph (with multiple
  /// devices)
  /// \param backward_connections_allowed Allow backward connections between
  /// devices (i.e. data can be sent from device 2 to device 1)
  /// \return The linearized graph (with multiple devices) and the map that
  /// associated every node id of the original graph to the respective node id
  /// of the "new" graph
  [[nodiscard]] std::pair<new_network, std::map<node_id_type, node_id_type>>
  block_graph(bool backward_connections_allowed) const
  {
    auto const num_of_devices = graph.get_num_devices();

    std::vector<Node<node_id_collection_type>> new_nodes;
    std::map<node_id_type, node_id_type> old_to_new; // Old node -> New node

    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      new_dependencies;

    auto const &old_dependencies = graph.get_dependencies();
    auto const &old_nodes        = graph.get_nodes();

    // Counter is used to establish if the current node has more output
    // connections than the inputs one.
    int counter = old_dependencies.front().second.size() -
                  old_dependencies.front().first.size() - 1;

    // Id of the node to be inserted
    node_id_type id = 0;

    // Add the source node with content equal to the id of the first node in the
    // original graph
    new_nodes.reserve(old_nodes.size());
    new_nodes.emplace_back(node_id_collection_type{});
    new_nodes.back().content = node_id_collection_type{0};

    old_to_new[old_nodes.begin()->get_id()] = id;
    ++id;

    // Cycle through all the nodes of the graph
    for (auto it = ++old_nodes.begin(); it != old_nodes.end(); ++it)
      {
        // Node of the all graph
        auto const &node          = *it;
        auto const &dep           = old_dependencies[node.get_id()];
        int const   local_counter = dep.second.size() - dep.first.size();

        // Add new node
        if (local_counter <= 0 && counter == 0)
          {
            new_nodes.emplace_back(node_id_collection_type{});
            new_nodes.back().content.insert(new_nodes.back().content.end(),
                                            node.get_id());

            old_to_new[node.get_id()] = id;

            ++id;
          }
        // Add new node and add master node for next steps
        else if (local_counter > 0 && counter == 0)
          {
            new_nodes.emplace_back(node_id_collection_type{});
            new_nodes.back().content.insert(new_nodes.back().content.end(),
                                            node.get_id());
            old_to_new[node.get_id()] = id;

            new_nodes.emplace_back(node_id_collection_type{});
            old_to_new[++id];

            counter += local_counter;
          }
        // Add node link to the "big" node
        else if ((local_counter == 0 && dep.second.size() == 1 ||
                  local_counter > 0 && dep.first.size() <= 1) &&
                 counter > 0)
          {
            new_nodes.back().content.insert(new_nodes.back().content.end(),
                                            node.get_id());

            counter += local_counter;

            old_to_new[node.get_id()] = id;
          }
        else if (counter > 0 && ((local_counter >= 0 && dep.first.size() > 1) ||
                                 (local_counter < 0)))
          {
            counter -= (dep.first.size() - 1);

            // End of the master node
            if (counter == 0)
              {
                new_nodes.emplace_back(node_id_collection_type{});
                old_to_new[node.get_id()] = ++id;
                new_nodes.back().content.insert(new_nodes.back().content.end(),
                                                node.get_id());

                // Do we have to add another master node?
                if (local_counter >= 0)
                  {
                    new_nodes.emplace_back(node_id_collection_type{});
                    old_to_new[++id];
                  }
                else
                  ++id;
              }
            else
              {
                old_to_new[node.get_id()] = id;
                new_nodes.back().content.insert(new_nodes.back().content.end(),
                                                node.get_id());
              }

            counter += (dep.second.size() - 1);
          }
        else
          {
            std::cout << "Unknown node found during block_graph construction"
                      << std::endl;
          }
      }

    auto const basic_size = new_nodes.size();
    auto const supp_size  = basic_size - 2;

    // Adjusting the indices
    for (auto &el : old_to_new)
      if (el.second >= 2)
        el.second = el.second * num_of_devices - (num_of_devices - 1);

    new_nodes.reserve(2 + supp_size * num_of_devices);
    new_dependencies.reserve(2 + supp_size * num_of_devices);

    // Add the other nodes. Their content is not needed since it can be
    // recovered from the nodes of the first device
    for (std::size_t k = 1; k < num_of_devices; ++k)
      for (std::size_t i = 1; i < basic_size - 1; ++i)
        {
          new_nodes.emplace_back(node_id_collection_type{});
          ++id;
        }

    // Move the content of the first nodes to their appropriate final position
    for (std::size_t i = basic_size - 1; i > 1; --i)
      {
        new_nodes[1 + num_of_devices * (i - 1)].content =
          std::move(new_nodes[i].content);
      }

    {
      new_dependencies.emplace_back(
        std::make_pair<node_id_collection_type, node_id_collection_type>({},
                                                                         {}));

      auto &out = new_dependencies.back().second;
      for (std::size_t k = 0; k < num_of_devices; ++k)
        out.insert(out.end(), k + 1);
    }

    {
      new_dependencies.emplace_back(
        std::make_pair<node_id_collection_type, node_id_collection_type>({0},
                                                                         {}));
      auto &out = new_dependencies.back().second;
      for (std::size_t k = 0; k < num_of_devices; ++k)
        out.insert(out.end(), 1 + num_of_devices + k);

      for (std::size_t k = 1; k < num_of_devices; ++k)
        {
          auto dep_cpy = new_dependencies.back();

          if (!backward_connections_allowed)
            dep_cpy.second.erase(num_of_devices + k);

          new_dependencies.push_back(std::move(dep_cpy));
        }
    }

    {
      for (std::size_t i = 2; i < supp_size; ++i)
        {
          auto const id = new_dependencies.size();
          new_dependencies.emplace_back(
            std::make_pair<node_id_collection_type, node_id_collection_type>(
              {}, {}));


          auto &in  = new_dependencies.back().first;
          auto &out = new_dependencies.back().second;

          if (!backward_connections_allowed)
            in.insert(in.end(), id - num_of_devices);


          for (std::size_t k = 0; k < num_of_devices; ++k)
            {
              if (backward_connections_allowed)
                in.insert(in.end(), id - num_of_devices + k);

              out.insert(out.end(), id + num_of_devices + k);
            }

          for (std::size_t k = 1; k < num_of_devices; ++k)
            {
              auto tmp_dep = new_dependencies.back();

              if (!backward_connections_allowed)
                {
                  tmp_dep.first.insert(id - num_of_devices + k);
                  tmp_dep.second.erase(id + num_of_devices + k - 1);
                }

              new_dependencies.emplace_back(std::move(tmp_dep));
            }
        }
    }

    {
      auto const id = new_dependencies.size();

      new_dependencies.emplace_back(
        std::make_pair<node_id_collection_type, node_id_collection_type>(
          {}, {id + num_of_devices}));

      auto &in = new_dependencies.back().first;
      in.insert(in.end(), id - num_of_devices);

      if (backward_connections_allowed)
        for (std::size_t k = 1; k < num_of_devices; ++k)
          in.insert(in.end(), id - num_of_devices + k);

      for (std::size_t k = 1; k < num_of_devices; ++k)
        {
          auto dep_cpy = new_dependencies.back();

          if (!backward_connections_allowed)
            dep_cpy.first.insert(id - num_of_devices + k);

          new_dependencies.emplace_back(std::move(dep_cpy));
        }
    }

    {
      new_dependencies.emplace_back(
        std::make_pair<node_id_collection_type, node_id_collection_type>({},
                                                                         {}));

      auto &in = new_dependencies.back().first;
      for (std::size_t k = 0; k < num_of_devices; ++k)
        in.insert(in.end(), new_dependencies.size() - 1 - num_of_devices + k);
    }

    return {new_network(new_nodes, new_dependencies), old_to_new};
  }

  /// Helper function used to estimate the memory usage of a group of nodes
  /// \param devices The devices
  /// \param constraint_type The type of the memory constraint
  /// \param ids The set of nodes to "analyze"
  /// \param input_memory The memory usage of all input nodes
  /// \param output_memory The memory usage of all output nodes
  /// \param params_memory The memory usage of all parameters nodes
  /// \return The pair of maximum memory of ios and of memory of parameters
  std::tuple<memory_type, memory_type>
  estimate_maximum_memory_usage(
    std::vector<Device> const      &devices,
    Memory_Constraint_Type          constraint_type,
    std::set<node_id_type> const   &ids,
    std::vector<memory_type> const &input_memory,
    std::vector<memory_type> const &output_memory,
    std::vector<memory_type> const &params_memory) const
  {
    memory_type result_memory = 0, fixed_memory = 0;

    if (constraint_type == Memory_Constraint_Type::Preload_Parameters)
      {
        fixed_memory =
          std::reduce(std::next(params_memory.begin(), *ids.cbegin()),
                      std::next(params_memory.begin(), *ids.crbegin()));
      }
    auto const &dependencies = graph.get_dependencies();
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

        if (constraint_type == Memory_Constraint_Type::Preload_Parameters)
          {
            result_memory = std::max(result_memory, in_memory + out_memory);
          }
        else if (constraint_type == Memory_Constraint_Type::Max)
          {
            result_memory =
              std::max(result_memory,
                       in_memory + out_memory + params_memory[id]);
          }

        if (children.size() > parents.size())
          qty = qty + dependencies[id].second.size() - parents.size() - 1;
      }

    return {result_memory * qty, fixed_memory};
  }

  // WIP
  memory_type
  remove_unfeasible_paths(std::vector<Device> const      &devices,
                          Memory_Constraint_Type          constraint_type,
                          std::set<node_id_type> const   &ids,
                          std::vector<memory_type> const &output_memory,
                          std::vector<memory_type> const &params_memory) const
  {
    memory_type current_memory_res = 0, real_memory_res = 0;

    if (constraint_type == Preload_Parameters)
      {
        current_memory_res = real_memory_res =
          std::reduce(std::next(params_memory.begin(), *ids.cbegin()),
                      std::next(params_memory.begin(), *ids.crbegin()));
      }

    std::map<node_id_type, std::set<node_id_type>> visited_ids;
    auto const &dependencies = graph.get_dependencies();

    for (auto const &id : ids)
      {
        auto const &node    = graph[id];
        auto const &parents = dependencies[id].first;

        for (auto const &parent : parents)
          {
            auto const it = visited_ids.find(parent);
            if (it != visited_ids.cend())
              {
                it->second.insert(id);
                if (it->second.size() == dependencies[parent].second.size())
                  {
                    current_memory_res -= output_memory[parent];
                  }
              }
          }

        if (constraint_type == Memory_Constraint_Type::Preload_Parameters)
          {
            real_memory_res = std::max(current_memory_res,
                                       current_memory_res += output_memory[id]);
          }
        else if (constraint_type == Memory_Constraint_Type::Max)
          {}

        visited_ids[id] = std::set<node_id_type>();
      }

    return current_memory_res;
  }

  /// Removes the "unfeasible" paths due to memory constraints
  /// \param devices The set of devices
  /// \param new_graph The linearized graph
  /// \param constraint_type The memory constraint
  void
  remove_unfeasible_paths(std::vector<Device> const &devices,
                          new_network               &new_graph,
                          Memory_Constraint_Type     constraint_type) const
  {
    if (constraint_type == Memory_Constraint_Type::None)
      return;

    auto const input_memory =
      Computer_memory::compute_nodes_memory_usage_input(graph);
    auto const output_memory =
      Computer_memory::compute_nodes_memory_usage_output(graph);
    auto const params_memory =
      Computer_memory::compute_nodes_memory_usage_parameters(graph);

    std::vector<bool> available(devices.size(), true);
    memory_type       memory_graph = 0;


    auto const dependencies_clear = [&](node_id_type const &node_id) {
      auto &dep = new_graph.get_dependencies_ref()[node_id];

      for (auto const &in : dep.first)
        new_graph.get_dependencies_ref()[in].second.erase(node_id);
      for (auto const &out : dep.second)
        new_graph.get_dependencies_ref()[out].first.erase(node_id);

      dep.first.clear();
      dep.second.clear();
    };

    auto const response_fun_max = [&](std::size_t        basic_node_id,
                                      memory_type const &memory_node) {
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
                }
            }
        }
    };

    auto const response_fun_preload_parameters = [&](std::size_t basic_node_id,
                                                     memory_type const &param,
                                                     memory_type const &io) {
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
            }
        }
    };

    for (std::size_t i = 1; i < new_graph.size() - 1; i += devices.size())
      {
        auto const &new_node_content = new_graph[i].content;
        bool        easy_content     = new_node_content.size() == 1;

        if (easy_content)
          {
            auto const index = *new_node_content.begin();

            if (constraint_type == Memory_Constraint_Type::Max)
              {
                response_fun_max(i,
                                 params_memory[index] + input_memory[index] +
                                   output_memory[index]);
              }
            else if (constraint_type ==
                     Memory_Constraint_Type::Preload_Parameters)
              {
                response_fun_preload_parameters(i,
                                                params_memory[index],
                                                input_memory[index] +
                                                  output_memory[index]);
              }
          }
        else
          {
            auto const [param_mem, io_mem] =
              estimate_maximum_memory_usage(devices,
                                            constraint_type,
                                            new_node_content,
                                            input_memory,
                                            output_memory,
                                            params_memory);

            if (constraint_type == Memory_Constraint_Type::Max)
              {
                response_fun_max(i, param_mem + io_mem);
              }
            else if (constraint_type ==
                     Memory_Constraint_Type::Preload_Parameters)
              {
                response_fun_preload_parameters(i, param_mem, io_mem);
              }
          }
      }
  }


  /// Given the current graph and the original weight function, it will produce
  /// the weigths for the linearized graph
  /// \param transmission_weights Used when we are switching from a device to
  /// another. The node is the source while the two size_t are the input and
  /// output device ids
  /// \param new_graph The lineatized graph (result of block_graph)
  void
  block_graph_weights(
    std::function<weight_type(node_id_type const &,
                              std::size_t,
                              std::size_t)> const &transmission_weights,
    new_network                                   &new_graph) const
  {
    auto const &nodes = new_graph.get_nodes();
    std::for_each(
      nodes.cbegin(),
      nodes.cend(),
      [&new_graph, &graph = graph, &transmission_weights](
        Node<node_id_collection_type> const &node) {
        auto const num_devices = graph.get_num_devices();
        auto const size  = (new_graph.get_nodes().size() - 2) / num_devices + 2;
        auto const first = node.get_id();

        // The index of the input node on the linearized graph
        auto const input_index =
          first == 0 ? 0 : first - (first - 1) % num_devices;

        for (auto const &second :
             new_graph.get_dependencies()[node.get_id()].second)
          {
            edge_type const edge = {first, second};
            auto const      output_index =
              edge.second == 0 ? 0 :
                                      edge.second - (edge.second - 1) % num_devices;

            // The device id of the input node (=0 starting device, >0 other
            // device)
            auto const in_device_id =
              edge.first == 0 ? 0 : (edge.first - 1) % num_devices;
            // The device id of the output node (=0 starting device, >0 other
            // device)
            auto const out_device_id =
              edge.second == 0 ? 0 : (edge.second - 1) % num_devices;

            // Look for the nodes of the original graph that are
            // represented by the output node (in the linearized
            // graph)
            auto const &outputs = new_graph.get_nodes()[output_index].content;

            // Look for the nodes of the original graph that are
            // represented by the input node (in the linearized
            // graph)
            auto const &inputs = new_graph.get_nodes()[input_index].content;

            double final_cost = -1.;

            // 1-1 correspondence
            if (outputs.size() == 1 && inputs.size() == 1)
              {
                auto const &input  = *inputs.begin();
                auto const &output = *outputs.begin();

                auto const tmp_edge = std::make_pair(input, output);

                auto const transmission_weight =
                  transmission_weights(input, in_device_id, out_device_id);
                auto const weight = graph.get_weigth(out_device_id, tmp_edge);

                final_cost = transmission_weight + weight;
              }
            // (2+)-1 correspondence. The idea is that the input nodes must
            // transmit to the output node the different values. Thus, the
            // transmission cost is paid serveral times.
            else if (outputs.size() == 1)
              {
                weight_type weight_cost        = .0;
                weight_type transmission_costs = .0;

                auto const &output       = *outputs.begin();
                auto const &dependencies = graph.get_dependencies();

                // The inputs on the original graph of the output node have to
                // transmit their values to the output node
                for (auto const &input : dependencies[output].first)
                  {
                    transmission_costs +=
                      transmission_weights(input, in_device_id, out_device_id);
                    weight_cost +=
                      graph.get_weigth(out_device_id,
                                       std::make_pair(input, output));
                  }

                final_cost = weight_cost + weight_cost;
              }
            // 1-(2+). In this case, the input is sent to the device of the
            // output nodes a single time. Thus, this transmission cost is taken
            // into account only once.
            else if (inputs.size() == 1)
              {
                weight_type weight_costs      = .0;
                weight_type transmission_cost = .0;

                auto const &dependencies = graph.get_dependencies();
                auto const &input        = *inputs.begin();
                auto const &comm_outputs = dependencies[*inputs.begin()].second;

                for (auto const &output : comm_outputs)
                  weight_costs +=
                    graph.get_weigth(out_device_id,
                                     std::make_pair(input, output));
                transmission_cost +=
                  transmission_weights(input, in_device_id, out_device_id);

                // Compute the total weight associated to the internal edges
                for (auto const &internal_input : outputs)
                  {
                    for (auto &internal_output :
                         dependencies[internal_input].second)
                      {
                        if (outputs.find(internal_output) != outputs.cend())
                          {
                            weight_costs +=
                              graph.get_weigth(out_device_id,
                                               std::make_pair(internal_input,
                                                              internal_output));
                          }
                      }
                  }

                final_cost = weight_costs + transmission_cost;
              }

            new_graph.set_weigth(edge, final_cost);
          }
      });
  }


  /// Returns the device id based on the node id
  /// \param node_id The node id
  /// \return The device id
  [[nodiscard]] std::size_t
  get_device_id(node_id_type const &node_id) const
  {
    return node_id == 0 ? 0 : (node_id - 1) % graph.get_num_devices();
  }

  /// Returns the id of the node with content in the linearized graph
  /// \param current_node_id The id of the node in the linearized graph
  /// \param device_id The device id
  /// \return The id of the node of the linearized graph with content
  [[nodiscard]] node_id_type
  get_base_node_id(node_id_type const &current_node_id,
                   std::size_t const  &device_id) const
  {
    return current_node_id == 0 ? 0 : current_node_id - device_id;
  }

  /// It will produce the set of nodes associated to a given device
  /// \param new_path The path
  /// \param new_graph The linearized graph
  /// \return The "real" path
  [[nodiscard]] Real_Path
  get_network_slices(path_info const   &new_path,
                     new_network const &new_graph) const
  {
    Real_Path   res;
    std::size_t current_model_device = 0;

    res.emplace_back(current_model_device, std::set<node_id_type>());

    auto const  new_graph_size = new_graph.get_nodes().size();
    auto const &path_nodes     = new_path.path;

    for (std::size_t i = 0; i < path_nodes.size(); ++i)
      {
        auto const  node_id_new_graph = path_nodes[i];
        std::size_t device_id         = get_device_id(node_id_new_graph);

        auto const base_node_id_new_graph =
          get_base_node_id(node_id_new_graph, device_id);

        if (device_id != current_model_device)
          {
            current_model_device = device_id;
            res.emplace_back(current_model_device, std::set<node_id_type>());
          }

        auto it_nodes = new_graph.get_nodes()[base_node_id_new_graph];
        res.back().second.insert(it_nodes.content.begin(),
                                 it_nodes.content.end());
      }

    return res;
  }

  /// It will produce the set of nodes associated to a given device for every
  /// new_path
  /// \param new_paths The collection of paths
  /// \param new_graph The
  /// linearized graph
  /// \return The "real" path
  [[nodiscard]] std::vector<Real_Path>
  get_network_slices(std::vector<path_info> const &new_paths,
                     new_network const            &new_graph) const
  {
    std::vector<Real_Path> res(new_paths.size());

    std::transform(new_paths.begin(),
                   new_paths.end(),
                   res.begin(),
                   [&new_graph, this](path_info const &path) {
                     return get_network_slices(path, new_graph);
                   });

    return res;
  }

  /// It will produce the set of nodes associated to a given device for every
  /// new_path with the associated path weight
  /// \param new_paths The collection of paths
  /// \param new_graph The
  /// linearized graph
  /// \return The "real" path
  [[nodiscard]] Weighted_Real_Paths
  get_weighted_network_slice(std::vector<path_info> const &new_paths,
                             new_network const            &new_graph) const
  {
    auto network_slice = get_network_slices(new_paths, new_graph);
    Weighted_Real_Paths final_res;

    final_res.reserve(network_slice.size());

    for (std::size_t i = 0; i < network_slice.size(); ++i)
      final_res.emplace_back(new_paths[i].length, std::move(network_slice[i]));
    return final_res;
  }

  /// It will produce the set of nodes associated to a given device for every
  /// new_path with the associated path weight
  /// \param new_paths The collection of paths
  /// \param network_slice The network slices
  /// \return The "real" weighted paths
  [[nodiscard]] Weighted_Real_Paths
  get_weighted_network_slice(
    std::vector<path_info> const &new_paths,
    std::vector<std::vector<std::pair<size_t, std::set<node_id_type>>>> const
      &network_slice) const
  {
    Weighted_Real_Paths final_res;
    final_res.reserve(network_slice.size());
    for (std::size_t i = 0; i < network_slice.size(); ++i)
      final_res.emplace_back(new_paths[i].length, network_slice[i]);
    return final_res;
  }

public:
  Butcher() = default;
  /// Move constructor
  explicit Butcher(network &&g)
    : graph(std::move(g)){};

  explicit Butcher(network const &g)
    : graph(g){};

  /// Basic getter for graph
  /// \return The graph (const reference)
  network const &
  get_graph() const
  {
    return graph;
  }

  /// Ref getter for graph
  /// \return Reference to the stored graph
  network &
  get_graph_ref()
  {
    return graph;
  }

  /// Check if a given partition satisfies the memory constraints
  /// \param real_path A real path
  /// \param devices Collection of devices
  /// \return True if the partition satisfies the memory constraint
  bool
  partition_memory_checker(Real_Path const                &real_path,
                           std::vector<Device> const &devices)
  {
    auto const nodes_memory_usage =
      Computer_memory::compute_nodes_memory_usage_output(graph, true);

    for (auto const &[model_id, nodes] : real_path)
      {
        memory_type memory = 0, branch_memory = 0, tmp_memory = 0;
        std::map<node_id_type, std::size_t> node_occupancy;

        for (auto const &node : nodes)
          {
            node_occupancy[node] = graph.get_dependencies()[node].second.size();
            tmp_memory           = nodes_memory_usage[node];

            for (auto const &ins : graph.get_dependencies()[node].first)
              {
                auto it = node_occupancy.find(ins);
                it->second -= 1;
                if (it->second == 0)
                  node_occupancy.erase(it);
              }

            if (node_occupancy.size() <= 1)
              {
                branch_memory = std::max(branch_memory, tmp_memory);
                memory        = std::max(memory, branch_memory);
                branch_memory = 0, tmp_memory = 0;
              }
            else
              {
                for (auto it = ++node_occupancy.crbegin();
                     it != node_occupancy.crend();
                     ++it)
                  {
                    tmp_memory += nodes_memory_usage[it->first];
                  }

                branch_memory = std::max(branch_memory, tmp_memory);
              }
          }

        branch_memory = std::max(branch_memory, tmp_memory);
        memory        = std::max(memory, branch_memory);

        if (memory > devices[model_id].maximum_memory)
          return false;
      }

    return true;
  }

  /// Computes the k shortest paths for the given graph
  /// \param transmission_weights The transmission weight function
  /// \param params The set of parameters
  /// \return The set of "best" paths
  Weighted_Real_Paths
  compute_k_shortest_path(
    std::function<weight_type(node_id_type const &,
                              std::size_t,
                              std::size_t)> const &transmission_weights,
    Parameters const                              &params) const
  {
    auto [new_graph, connection_map] =
      block_graph(params.backward_connections_allowed);


    if (params.memory_constraint != Memory_Constraint_Type::None &&
        !params.backward_connections_allowed)
      remove_unfeasible_paths(params.devices,
                              new_graph,
                              params.memory_constraint_type);


    block_graph_weights(transmission_weights, new_graph);

    std::unique_ptr<KFinder<new_network>> kFinder;

    switch (params.method)
      {
        case KSP_Method::Eppstein:
          kFinder = std::make_unique<KFinder_Eppstein<new_network>>(new_graph);
          break;
        case KSP_Method::Lazy_Eppstein:
          kFinder =
            std::make_unique<KFinder_Lazy_Eppstein<new_network>>(new_graph);
          break;
        default:
          return {};
      }

    auto const res = kFinder->compute(params.K);
    return get_weighted_network_slice(res, new_graph);
  }
};

#endif // NETWORK_BUTCHER_BUTCHER_H