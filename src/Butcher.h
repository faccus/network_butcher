//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_BUTCHER_H
#define NETWORK_BUTCHER_BUTCHER_H

#include <algorithm>


#include "Helpers/Traits/Graph_traits.h"
#include "Helpers/Traits/Hardware_traits.h"

#include "Helpers/Computer/Computer_memory.h"

#include "Network/Graph.h"
#include "Network/Node.h"

#include "Helpers/K-shortest_path/KEppstein.h"
#include "Helpers/K-shortest_path/KEppstein_lazy.h"

#include "Helpers/Types/Type_info.h"

#include "Helpers/Utilities.h"


using real_path = std::vector<std::pair<std::size_t, std::set<node_id_type>>>;
using weighted_real_path = std::vector<std::pair<real_path, weight_type>>;

/// Butcher butchers a given graph into slices
template <class T>
class Butcher
{
private:
  using network     = Graph<T>;
  using new_network = Graph<node_id_collection_type>;

  network graph;

  /// Compute the minimal connected graphs (with dependencies) containing every
  /// node
  /// \return returns the smallest connected sub-graph (with dependencies)
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
  /// \param slices The vector of input slices
  /// \param tester The tester function
  /// \return The slices that satisfy the tester function
  static std::vector<slice_type>
  partition_checker(std::vector<slice_type>                       &slices,
                    const std::function<bool(const slice_type &)> &tester)
  {
    std::vector<slice_type> res;

    for (int i = 0; i < slices.size(); ++i)
      {
        if (tester(slices[i]))
          res.push_back(slices[i]);
      }

    return res;
  };


  /// It will produce a linearized version of the current graph (with multiple
  /// devices)
  /// \return The linearized graph (with multiple devices) and the map that
  /// associated every node id of the original graph to the respective node id
  /// of the "new" graph
  [[nodiscard]] std::pair<new_network, std::map<node_id_type, node_id_type>>
  block_graph(std::size_t num_of_devices = 1) const
  {
    std::vector<Node<node_id_collection_type>> new_nodes;
    std::map<node_id_type, node_id_type> new_content; // Old node -> New node
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      new_dependencies;

    // Counter is used to establish if the current node has more output
    // connections than the inputs one.
    int counter = graph.dependencies.front().second.size() -
                  graph.dependencies.front().first.size() - 1;

    // Id of the node to be inserted
    node_id_type id = 0;

    // Add the source node with content equal to the id of the first node in the
    // original graph
    new_nodes.reserve(graph.nodes.size());
    new_nodes.emplace_back();
    ++id;
    new_nodes.back().content = node_id_collection_type{0};

    // Cycle through all the nodes of the graph
    for (auto it = ++graph.nodes.begin(); it != graph.nodes.end(); ++it)
      {
        // Node of the all graph
        auto const &node          = *it;
        auto const &dep           = graph.dependencies[node.get_id()];
        int const   local_counter = dep.second.size() - dep.first.size();

        // Add new node
        if (local_counter <= 0 && counter == 0)
          {
            new_nodes.emplace_back();
            new_nodes.back().content.insert(new_nodes.back().content.end(), id);

            new_content[node.get_id()] = id;

            ++id;
          }
        // Add new node and add master node for next steps
        else if (local_counter > 0 && counter == 0)
          {
            new_nodes.emplace_back();
            new_nodes.back().content.insert(new_nodes.back().content.end(), id);
            new_content[node.get_id()] = id;

            new_nodes.emplace_back();
            new_content[++id];

            counter += local_counter;
          }
        // Add node link to the "big" node
        else if ((local_counter == 0 && dep.second.size() == 1 ||
                  local_counter > 0 && dep.first.size() <= 1) &&
                 counter > 0)
          {
            new_nodes.back().content.insert(new_nodes.back().content.end(), id);
            new_content[node.get_id()] = id;
          }
        else if (counter > 0 && ((local_counter >= 0 && dep.first.size() > 1) ||
                                 (local_counter < 0)))
          {
            counter -= (dep.first.size() - 1);

            // End of the master node
            if (counter == 0)
              {
                new_nodes.emplace_back();
                new_content[node.get_id()] = ++id;
                new_nodes.back().content.insert(new_nodes.back().content.end(),
                                                id);

                // Do we have to add another master node?
                if (local_counter >= 0)
                  {
                    new_nodes.emplace_back();
                    new_content[++id];
                  }
                else
                  ++id;
              }
            else
              {
                new_content[node.get_id()] = id;
                new_nodes.back().content.insert(new_nodes.back().content.end(),
                                                id);
              }

            counter += (dep.second.size() - 1);
          }
        else
          {
            std::cout << std::endl;
          }
      }

    auto const basic_size = new_nodes.size();
    auto const supp_size  = basic_size - 2;

    // Adjusting the indices
    for (auto &el : new_content)
      if (el.second >= 2)
        el.second = el.second * num_of_devices - (num_of_devices - 1);

    new_nodes.reserve(2 + supp_size * num_of_devices);
    new_dependencies.reserve(2 + supp_size * num_of_devices);

    // Add the other nodes. Their content is not needed since it can be
    // recovered from the nodes of the first device
    for (std::size_t k = 1; k < num_of_devices; ++k)
      for (std::size_t i = 1; i < basic_size - 1; ++i)
        {
          new_nodes.emplace_back();
          id++;
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
        new_dependencies.push_back(new_dependencies.back());
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

          for (std::size_t k = 0; k < num_of_devices; ++k)
            {
              in.insert(in.end(), id - num_of_devices + k);
              out.insert(out.end(), id + num_of_devices + k);
            }

          for (std::size_t k = 1; k < num_of_devices; ++k)
            new_dependencies.push_back(new_dependencies.back());
        }
    }

    {
      auto const id = new_dependencies.size();

      new_dependencies.emplace_back(
        std::make_pair<node_id_collection_type, node_id_collection_type>(
          {}, {id + num_of_devices}));

      auto &in = new_dependencies.back().first;
      for (std::size_t k = 0; k < num_of_devices; ++k)
        in.insert(in.end(), id - num_of_devices + k);
      for (std::size_t k = 1; k < num_of_devices; ++k)
        new_dependencies.push_back(new_dependencies.back());
    }

    {
      new_dependencies.emplace_back(
        std::make_pair<node_id_collection_type, node_id_collection_type>({},
                                                                         {}));

      auto &in = new_dependencies.back().first;
      for (std::size_t k = 0; k < num_of_devices; ++k)
        in.insert(in.end(), new_dependencies.size() - 1 - num_of_devices + k);
    }

    return {Graph(new_nodes, new_dependencies), new_content};
  }


  /// Given the current graph and the original weight function, it will produce
  /// a new weight function for the linearized graph
  /// \param original_weights The weight function for the original graph and for
  /// the different devices
  /// \param transmission_weights Used when we are switching from a device to
  /// another. The node is the source while the two size_t are the input and
  /// output device ids
  /// \param new_graph The lineatized graph (result of block_graph)
  void
  block_graph_weights(
    std::vector<std::function<weight_type(edge_type const &)>>
      &original_weights,
    std::function<weight_type(node_id_type const &, std::size_t, std::size_t)>
                &transmission_weights,
    new_network &new_graph) const
  {
    return [&transmission_weights,
            &original_weights,
            &new_graph,
            &graph = graph](edge_type const &edge) {
      auto &new_weight_map = new_graph.weigth_map;
      // If a weight was created, return it
      auto const candidate_sol = new_weight_map.find(edge);
      if (candidate_sol != new_weight_map.cend())
        return candidate_sol->second;

      auto const num_devices = original_weights.size();
      auto const size = (new_graph.get_nodes().size() - 2) / num_devices + 2;

      // The index of the input node on the linearized graph
      auto const input_index =
        edge.first == 0 ? 0 : edge.first - (edge.first - 1) % num_devices;

      // The index of the output node on the linearized graph
      auto const output_index =
        edge.second == 0 ? 0 : edge.second - (edge.second - 1) % num_devices;


      if (input_index >= output_index ||
          input_index == 0 && output_index != 1 ||
          input_index != 0 && output_index - input_index != num_devices)
        {
          std::cout
            << "Requested an invalid edge: check if the graph is correct!"
            << std::endl;
          return -1.;
        }

      // The device id of the input node (=0 starting device, >0 other device)
      auto const in_device_id =
        edge.first == 0 ? 0 : (edge.first - 1) % num_devices;
      // The device id of the output node (=0 starting device, >0 other device)
      auto const out_device_id =
        edge.second == 0 ? 0 : (edge.second - 1) % num_devices;


      // Look for the nodes of the original graph that are represented by the
      // output node (in the linearized graph)
      auto const &output_node_rep = new_graph.get_nodes()[output_index];

      if (output_node_rep.content.empty())
        return -1.;

      // Look for the nodes of the original graph that are represented by the
      // input node (in the linearized graph)
      auto const &input_node_rep = new_graph.get_nodes()[input_index];
      if (input_node_rep.content.empty())
        return -1.;

      auto const &inputs  = input_node_rep.content;
      auto const &outputs = output_node_rep.content;

      double final_cost = -1.;

      // 1-1 correspondence
      if (outputs.size() == 1 && inputs.size() == 1)
        {
          auto const &input  = *inputs.begin();
          auto const &output = *outputs.begin();

          auto const tmp_edge = std::make_pair(input, output);

          auto const transmission_weight =
            transmission_weights(input, in_device_id, out_device_id);
          auto const weight = original_weights[out_device_id](tmp_edge);

          final_cost                 = transmission_weight + weight;
          new_graph.weigth_map[edge] = final_cost;
        }
      // (2+)-1 correspondence. The idea is that the input nodes must transmit
      // to the output node the different values. Thus, the transmission cost is
      // paid serveral times.
      else if (outputs.size() == 1)
        {
          weight_type weight_cost        = .0;
          weight_type transmission_costs = .0;

          auto const &output = *outputs.begin();

          // The inputs on the original graph of the output node have to
          // transmit their values to the output node
          for (auto const &input : graph.dependencies[output].first)
            {
              transmission_costs +=
                transmission_weights(input, in_device_id, out_device_id);
              weight_cost +=
                original_weights[out_device_id](std::make_pair(input, output));
            }

          final_cost                 = weight_cost + weight_cost;
          new_graph.weigth_map[edge] = final_cost;
        }
      // 1-(2+). In this case, the input is sent to the device of the output
      // nodes a single time. Thus, this transmission cost is taken into account
      // only once.
      else if (inputs.size() == 1)
        {
          weight_type weight_costs      = .0;
          weight_type transmission_cost = .0;

          auto const &input        = *inputs.begin();
          auto const &comm_outputs = graph.dependencies[*inputs.begin()].second;

          for (auto const &output : comm_outputs)
            weight_costs +=
              original_weights[out_device_id](std::make_pair(input, output));
          transmission_cost +=
            transmission_weights(input, in_device_id, out_device_id);

          // Compute the total weight associated to the internal edges
          for (auto const &internal_input : outputs)
            {
              for (auto &internal_output :
                   graph.dependencies[internal_input].second)
                {
                  if (outputs.find(internal_output) != outputs.cend())
                    {
                      weight_costs += original_weights[out_device_id](
                        std::make_pair(internal_input, internal_output));
                    }
                }
            }

          final_cost                 = weight_costs + transmission_cost;
          new_graph.weigth_map[edge] = final_cost;
        }

      return final_cost;
    };
  }

  [[nodiscard]] std::size_t
  get_num_devices(std::size_t const &original_graph_size,
                  std::size_t const &new_graph_size) const
  {
    if (original_graph_size > 2)
      return (new_graph_size - 2) / (original_graph_size - 2);
    return 1;
  }

  [[nodiscard]] std::size_t
  get_device_id(node_id_type const &node_id,
                std::size_t const  &num_devices) const
  {
    return node_id == 0 ? 0 : (node_id - 1) % num_devices;
  }

  [[nodiscard]] node_id_type
  get_base_node_id(node_id_type const &current_node_id,
                   std::size_t const  &device_id) const
  {
    return current_node_id == 0 ? 0 : current_node_id - device_id;
  }

  [[nodiscard]] real_path
  get_network_slices(path_info const   &new_path,
                     new_network const &new_graph) const
  {
    real_path   res;
    std::size_t current_model_device = 0;

    res.emplace_back(current_model_device, std::set<node_id_type>());

    auto const  new_graph_size = new_graph.get_nodes().size();
    auto const &path_nodes     = new_path.path;

    auto const num_devices =
      get_num_devices(path_nodes.size() - 2, new_graph_size);

    for (std::size_t i = 0; i < path_nodes.size(); ++i)
      {
        auto const  node_id_new_graph = path_nodes[i];
        std::size_t device_id = get_device_id(node_id_new_graph, num_devices);

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

  [[nodiscard]] std::vector<real_path>
  get_network_slices(std::vector<path_info> const &new_paths,
                     new_network const            &new_graph) const
  {
    std::vector<real_path> res(new_paths.size());

    std::transform(new_paths.begin(),
                   new_paths.end(),
                   res.begin(),
                   [&new_graph, this](path_info const &path) {
                     return get_network_slices(path, new_graph);
                   });

    return res;
  }

  [[nodiscard]] weighted_real_path
  get_weighted_network_slice(std::vector<path_info> const &new_paths,
                             new_network const            &new_graph) const
  {
    auto               network_slice = get_network_slices(new_paths, new_graph);
    weighted_real_path final_res;

    final_res.reserve(network_slice.size());

    for (std::size_t i = 0; i < network_slice.size(); ++i)
      final_res.emplace_back(std::move(network_slice[i]), new_paths[i].length);
    return final_res;
  }

  [[nodiscard]] weighted_real_path
  get_weighted_network_slice(
    std::vector<path_info> const &new_paths,
    std::vector<std::vector<std::pair<size_t, std::set<node_id_type>>>> const
      &network_slice) const
  {
    weighted_real_path final_res;
    final_res.reserve(network_slice.size());
    for (std::size_t i = 0; i < network_slice.size(); ++i)
      final_res.emplace_back(network_slice[i], new_paths[i].length);
    return final_res;
  }

public:
  Butcher() = default;
  /// Move constructor
  explicit Butcher(network &&g)
    : graph(std::move(g)){};

  /// Basic getter for graph
  /// \return The graph (const reference)
  const network &
  getGraph() const
  {
    return graph;
  }


  /// It will compute every possible 2-slice partition of the network and it
  /// will select the partition whose total memory usage is less than the
  /// specified value
  /// \param memory_first_slice Total memory usage allowed to the first slice
  /// \return a collection of all the admissible partitions (and the nodes
  /// contained in the first partition)
  [[nodiscard]] std::vector<slice_type>
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
  [[nodiscard]] std::vector<slice_type>
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


  /// It will prodice the k-shortest paths for the linearized block graph
  /// associated with the original one
  /// \param weights The vector of weight map function, that associates to every
  /// edge the corresponding weight for the corresponding device
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The k-shortest paths on the graph (with the lenghts and devices)
  weighted_real_path
  compute_k_shortest_paths_eppstein_linear(
    std::vector<std::function<weight_type(edge_type const &)>> &weights,
    std::function<weight_type(node_id_type const &, std::size_t, std::size_t)>
               &transmission_weights,
    std::size_t num_of_devices,
    std::size_t k) const
  {
    auto const new_graph = block_graph(num_of_devices);

    block_graph_weights(weights, transmission_weights, new_graph);

    KFinder_Eppstein kFinder(new_graph);
    auto const       res = kFinder.eppstein(k);

    return get_weighted_network_slice(res, new_graph);
  }


  /// It will prodice the k-shortest paths for the linearized block graph
  /// associated with the original one
  /// \param weights The vector of weight map function, that associates to every
  /// edge the corresponding weight for the corresponding device
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The k-shortest paths on the graph (with the lenghts and devices)
  weighted_real_path
  compute_k_shortest_paths_lazy_eppstein_linear(
    std::vector<std::function<weight_type(edge_type const &)>> &weights,
    std::function<weight_type(node_id_type const &, std::size_t, std::size_t)>
               &transmission_weights,
    std::size_t num_of_devices,
    std::size_t k) const
  {
    auto const new_graph = block_graph(num_of_devices);

    block_graph_weights(weights, transmission_weights, new_graph);

    KFinder_Lazy_Eppstein kFinder(new_graph);

    auto const res = kFinder.lazy_eppstein(k);

    return get_weighted_network_slice(res, new_graph);
  }
};

#endif // NETWORK_BUTCHER_BUTCHER_H