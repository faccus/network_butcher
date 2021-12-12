//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_BUTCHER_H
#define NETWORK_BUTCHER_BUTCHER_H

#include "Helpers/Traits/Graph_traits.h"
#include "Helpers/Traits/Hardware_traits.h"

#include "Helpers/Computer/Computer_memory.h"

#include "Network/Graph.h"
#include "Network/Node.h"

#include "Helpers/K-shortest_path/KEppstein.h"
#include "Helpers/K-shortest_path/KEppstein_lazy.h"

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
  /// \return The linearized graph (with multiple devices)
  [[nodiscard]] Graph<node_id_type, node_id_type>
  block_graph(std::size_t num_of_devices = 1) const
  {
    std::vector<node_type>               new_nodes;
    std::map<node_id_type, node_id_type> new_content; // Old node -> New node
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      new_dependencies;

    // Counter is used to establish if the current node has more output
    // connections than the inputs one.
    int counter = graph.dependencies.front().second.size() -
                  graph.dependencies.front().first.size() - 1;

    // Id of the node to be inserted
    std::size_t id = 0;

    new_nodes.reserve(graph.nodes.size());
    new_nodes.emplace_back();
    id++;
    new_content[0] = 0;

    for (auto it = ++graph.nodes.begin(); it != graph.nodes.end(); ++it)
      {
        auto const &node          = *it;
        auto const &dep           = graph.dependencies[node.get_id()];
        int const   local_counter = dep.second.size() - dep.first.size();

        // Add new node
        if (local_counter <= 0 && counter == 0)
          {
            new_nodes.emplace_back();
            new_content[node.get_id()] = id;

            ++id;
          }
        // Add new node and add master node for next steps
        else if (local_counter > 0 && counter == 0)
          {
            new_nodes.emplace_back();
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

    for (auto &el : new_content)
      if (el.second >= 2)
        el.second = el.second * num_of_devices - (num_of_devices - 1);

    new_nodes.reserve(2 + supp_size * num_of_devices);
    new_dependencies.reserve(2 + supp_size * num_of_devices);

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

    return Graph(new_nodes, new_content, new_dependencies, false);
  }


  /// Given the current graph and the original weight function, it will produce
  /// a new weight function for the linearized graph
  /// \param new_weight_map The new weight map will look for the value of the
  /// weight firstly in this map. If it cannot find it, it will store the proper
  /// weight into the map (to make everything faster)
  /// \param original_weights The weight function for the original graph and for
  /// the different devices
  /// \param transmission_weights Used when we are switching from a device to
  /// another. The node is the source while the two size_t are the input and
  /// output device ids
  /// \param new_graph The lineatized graph (result of block_graph)
  /// \return The new weight function
  [[nodiscard]] std::function<type_weight(edge_type const &)>
  block_graph_weights(
    type_collection_weights &new_weight_map,
    std::vector<std::function<type_weight(edge_type const &)>>
      &original_weights,
    std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
                                            &transmission_weights,
    Graph<node_id_type, node_id_type> const &new_graph) const
  {
    std::unordered_map<node_id_type, std::set<node_id_type>>
      map; // New node -> Old nodes

    for (auto const &node : new_graph.nodes_content)
      map[node.second].insert(node.first);

    return [&new_graph,
            &original_weights,
            &new_weight_map,
            &transmission_weights,
            &graph = graph,
            map](edge_type const &edge) {
      // If a weight was created, return it
      auto const candidate_sol = new_weight_map.find(edge);
      if (candidate_sol != new_weight_map.cend())
        return candidate_sol->second;

      auto const num_devices = original_weights.size();
      auto const size        = (new_graph.nodes.size() - 2) / num_devices + 2;

      // The index of the input node on the linearized graph
      auto const first_index =
        edge.first == 0 ? 0 : edge.first - (edge.first - 1) % num_devices;

      // The index of the output node on the linearized graph
      auto const second_index =
        edge.second == 0 ? 0 : edge.second - (edge.second - 1) % num_devices;


      if (first_index >= second_index ||
          first_index == 0 && second_index != 1 ||
          first_index != 0 && second_index - first_index != num_devices)
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
      auto const it_out = map.find(second_index);

      if (it_out == map.cend() || it_out->second.size() == 0)
        return -1.;

      // Look for the nodes of the original graph that are represented by the
      // input node (in the linearized graph)
      auto const it_in = map.find(first_index);
      if (it_in == map.cend() || it_in->second.size() == 0)
        return -1.;

      auto const &inputs  = it_in->second;
      auto const &outputs = it_out->second;

      // 1-1 correspondence
      if (outputs.size() == 1 && inputs.size() == 1)
        {
          auto const &input  = *inputs.begin();
          auto const &output = *outputs.begin();

          auto const tmp_edge = std::make_pair(input, output);

          auto const transmission_weight =
            transmission_weights(input, in_device_id, out_device_id);
          auto const weight = original_weights[out_device_id](tmp_edge);

          auto const total_cost = transmission_weight + weight;
          new_weight_map[edge]  = total_cost;

          return total_cost;
        }
      // (2+)-1 correspondence. The idea is that the input nodes must transmit
      // to the output node the different values. Thus, the transmission cost is
      // paid serveral times.
      else if (outputs.size() == 1)
        {
          type_weight res    = .0;
          auto const &output = *outputs.begin();

          // The inputs on the original graph of the output node have to
          // transmit their values to the output node
          for (auto const &input : graph.dependencies[output].first)
            {
              res += transmission_weights(input, in_device_id, out_device_id);
              res +=
                original_weights[out_device_id](std::make_pair(input, output));
            }

          new_weight_map[edge] = res;
          return res;
        }
      // 1-(2+). In this case, the input is sent to the device of the output
      // nodes a single time. Thus, this transmission cost is taken into account
      // only once.
      else if (inputs.size() == 1)
        {
          type_weight res          = .0;
          auto const &input        = *inputs.begin();
          auto const &comm_outputs = graph.dependencies[*inputs.begin()].second;

          for (auto const &output : comm_outputs)
            res +=
              original_weights[out_device_id](std::make_pair(input, output));
          res += transmission_weights(input, in_device_id, out_device_id);

          // Compute the total weight associated to the internal edges
          for (auto const &internal_input : outputs)
            {
              for (auto &internal_output :
                   graph.dependencies[internal_input].second)
                {
                  if (outputs.find(internal_output) != outputs.cend())
                    {
                      res += original_weights[out_device_id](
                        std::make_pair(internal_input, internal_output));
                    }
                }
            }

          new_weight_map[edge] = res;
          return res;
        }
      else
        return -1.;
    };
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


  /// It will prodice the k-shortest paths for the linearized block graph
  /// associated with the original one
  /// \param weights The vector of weight map function, that associates to every
  /// edge (also the "fake" ones) the corresponding weight for the corresponding
  /// device
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The auxiliary graph and the k-shortest paths on the auxiliary
  /// graph
  std::pair<Graph<node_id_type, node_id_type>, std::vector<path_info>>
  compute_k_shortest_paths_eppstein_linear(
    std::vector<std::function<type_weight(edge_type const &)>> &weights,
    std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
               &transmission_weights,
    std::size_t num_of_devices,
    std::size_t k) const
  {
    auto const              new_graph = block_graph(num_of_devices);
    type_collection_weights new_weight_map;

    auto new_weights_fun = block_graph_weights(new_weight_map,
                                               weights,
                                               transmission_weights,
                                               new_graph);

    KFinder_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res = kFinder.eppstein(new_weights_fun, k);

    return {new_graph, res};
  }


  /// It will prodice the k-shortest paths for the linearized block graph
  /// associated with the original one
  /// \param weights The vector of weight map function, that associates to every
  /// edge (also the "fake" ones) the corresponding weight for the corresponding
  /// device
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The auxiliary graph and the k-shortest paths on the auxiliary
  /// graph
  std::pair<Graph<node_id_type, node_id_type>, std::vector<path_info>>
  compute_k_shortest_paths_lazy_eppstein_linear(
    std::vector<std::function<type_weight(edge_type const &)>> &weights,
    std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
               &transmission_weights,
    std::size_t num_of_devices,
    std::size_t k) const
  {
    auto const              new_graph = block_graph(num_of_devices);
    type_collection_weights new_weight_map;

    auto new_weights_fun = block_graph_weights(new_weight_map,
                                               weights,
                                               transmission_weights,
                                               new_graph);

    KFinder_Lazy_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res = kFinder.lazy_eppstein(new_weights_fun, k);

    return {new_graph, res};
  }
};


/// Butcher butchers a given graph into slices
template <>
class Butcher<graph_input_type>
{
private:
  using network = Graph<graph_input_type>;

  network          graph;


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
  /// \return The linearized graph (with multiple devices)
  [[nodiscard]] Graph<node_id_type, node_id_type>
  block_graph(std::size_t num_of_devices = 1) const
  {
    std::vector<node_type>               new_nodes;
    std::map<node_id_type, node_id_type> new_content; // Old node -> New node
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      new_dependencies;

    // Counter is used to establish if the current node has more output
    // connections than the inputs one.
    int counter = graph.dependencies.front().second.size() -
                  graph.dependencies.front().first.size() - 1;

    // Id of the node to be inserted
    std::size_t id = 0;

    new_nodes.reserve(graph.nodes.size());
    new_nodes.emplace_back();
    id++;
    new_content[0] = 0;

    for (auto it = ++graph.nodes.begin(); it != graph.nodes.end(); ++it)
      {
        auto const &node          = *it;
        auto const &dep           = graph.dependencies[node.get_id()];
        int const   local_counter = dep.second.size() - dep.first.size();

        // Add new node
        if (local_counter <= 0 && counter == 0)
          {
            new_nodes.emplace_back();
            new_content[node.get_id()] = id;

            ++id;
          }
        // Add new node and add master node for next steps
        else if (local_counter > 0 && counter == 0)
          {
            new_nodes.emplace_back();
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

    for (auto &el : new_content)
      if (el.second >= 2)
        {
          el.second *= num_of_devices;
          el.second -= (num_of_devices - 1);
        }

    new_nodes.reserve(2 + supp_size * num_of_devices);
    new_dependencies.reserve(2 + supp_size * num_of_devices);

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

    return Graph(new_nodes, new_content, new_dependencies, false);
  }


  /// Given the current graph and the original weight function, it will produce
  /// a new weight function for the linearized graph
  /// \param new_weight_map The new weight map will look for the value of the
  /// weight firstly in this map. If it cannot find it, it will store the proper
  /// weight into the map (to make everything faster)
  /// \param original_weights The weight function for the original graph and for
  /// the different devices
  /// \param transmission_weights Used when we are switching from a device to
  /// another. The node is the source while the two size_t are the input and
  /// output device ids
  /// \param new_graph The lineatized graph (result of block_graph)
  /// \return The new weight function
  [[nodiscard]] std::function<type_weight(edge_type const &)>
  block_graph_weights(
    type_collection_weights &new_weight_map,
    std::vector<std::function<type_weight(edge_type const &)>>
      &original_weights,
    std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
                                            &transmission_weights,
    Graph<node_id_type, node_id_type> const &new_graph) const
  {
    std::unordered_map<node_id_type, std::set<node_id_type>>
      map; // New node -> Old nodes

    for (auto const &node : new_graph.nodes_content)
      map[node.second].insert(node.first);

    return [&new_graph,
            &original_weights,
            &new_weight_map,
            &transmission_weights,
            &graph = graph,
            map](edge_type const &edge) {
      // If a weight was created, return it
      auto const candidate_sol = new_weight_map.find(edge);
      if (candidate_sol != new_weight_map.cend())
        return candidate_sol->second;

      auto const num_devices = original_weights.size();
      auto const size        = (new_graph.nodes.size() - 2) / num_devices + 2;

      // The index of the input node on the linearized graph
      auto const first_index =
        edge.first == 0 ? 0 : edge.first - (edge.first - 1) % num_devices;

      // The index of the output node on the linearized graph
      auto const second_index =
        edge.second == 0 ? 0 : edge.second - (edge.second - 1) % num_devices;


      if (first_index >= second_index ||
          first_index == 0 && second_index != 1 ||
          first_index != 0 && second_index - first_index != num_devices)
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
      auto const it_out = map.find(second_index);

      if (it_out == map.cend() || it_out->second.size() == 0)
        return -1.;

      // Look for the nodes of the original graph that are represented by the
      // input node (in the linearized graph)
      auto const it_in = map.find(first_index);
      if (it_in == map.cend() || it_in->second.size() == 0)
        return -1.;

      auto const &inputs  = it_in->second;
      auto const &outputs = it_out->second;

      // 1-1 correspondence
      if (outputs.size() == 1 && inputs.size() == 1)
        {
          auto const &input  = *inputs.begin();
          auto const &output = *outputs.begin();

          auto const tmp_edge = std::make_pair(input, output);

          auto const transmission_weight =
            transmission_weights(input, in_device_id, out_device_id);
          auto const weight = original_weights[out_device_id](tmp_edge);

          auto const total_cost = transmission_weight + weight;
          new_weight_map[edge]  = total_cost;

          return total_cost;
        }
      // (2+)-1 correspondence. The idea is that the input nodes must transmit
      // to the output node the different values. Thus, the transmission cost is
      // paid serveral times.
      else if (outputs.size() == 1)
        {
          type_weight res    = .0;
          auto const &output = *outputs.begin();

          // The inputs on the original graph of the output node have to
          // transmit their values to the output node
          for (auto const &input : graph.dependencies[output].first)
            {
              res += transmission_weights(input, in_device_id, out_device_id);
              res +=
                original_weights[out_device_id](std::make_pair(input, output));
            }

          new_weight_map[edge] = res;
          return res;
        }
      // 1-(2+). In this case, the input is sent to the device of the output
      // nodes a single time. Thus, this transmission cost is taken into account
      // only once.
      else if (inputs.size() == 1)
        {
          type_weight res          = .0;
          auto const &input        = *inputs.begin();
          auto const &comm_outputs = graph.dependencies[*inputs.begin()].second;

          for (auto const &output : comm_outputs)
            res +=
              original_weights[out_device_id](std::make_pair(input, output));
          res += transmission_weights(input, in_device_id, out_device_id);

          // Compute the total weight associated to the internal edges
          for (auto const &internal_input : outputs)
            {
              for (auto &internal_output :
                   graph.dependencies[internal_input].second)
                {
                  if (outputs.find(internal_output) != outputs.cend())
                    {
                      res += original_weights[out_device_id](
                        std::make_pair(internal_input, internal_output));
                    }
                }
            }

          new_weight_map[edge] = res;
          return res;
        }
      else
        return -1.;
    };
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

  /// Read from a file the model, construct the associated graph and prepare the
  /// butcher
  /// \param p Full path to the .onnx file model
  /// \param ignore_parameters Allows to choose if graph should ignore already
  /// initialized inputs/outputs (parameters)
  Butcher(const std::string &p, bool ignore_parameters = false)
    : graph(p, ignore_parameters){};


  /// It will compute every possible 2-slice partition of the network and it
  /// will select the partition whose total memory usage is less than the
  /// specified value
  /// \param memory_first_slice Total memory usage allowed to the first slice
  /// \return a collection of all the admissible partitions (and the nodes
  /// contained in the first partition)
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


  /// It will prodice the k-shortest paths for the linearized block graph
  /// associated with the original one
  /// \param weights The vector of weight map function, that associates to every
  /// edge (also the "fake" ones) the corresponding weight for the corresponding
  /// device
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The auxiliary graph and the k-shortest paths on the auxiliary
  /// graph
  std::pair<Graph<node_id_type, node_id_type>, std::vector<path_info>>
  compute_k_shortest_paths_eppstein_linear(
    std::vector<std::function<type_weight(edge_type const &)>> &weights,
    std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
               &transmission_weights,
    std::size_t num_of_devices,
    std::size_t k) const
  {
    auto const              new_graph = block_graph(num_of_devices);
    type_collection_weights new_weight_map;

    auto new_weights_fun = block_graph_weights(new_weight_map,
                                               weights,
                                               transmission_weights,
                                               new_graph);

    KFinder_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res = kFinder.eppstein(new_weights_fun, k);

    return {new_graph, res};
  }


  /// It will prodice the k-shortest paths for the linearized block graph
  /// associated with the original one
  /// \param weights The vector of weight map function, that associates to every
  /// edge (also the "fake" ones) the corresponding weight for the corresponding
  /// device
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The auxiliary graph and the k-shortest paths on the auxiliary
  /// graph
  std::pair<Graph<node_id_type, node_id_type>, std::vector<path_info>>
  compute_k_shortest_paths_lazy_eppstein_linear(
    std::vector<std::function<type_weight(edge_type const &)>> &weights,
    std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
               &transmission_weights,
    std::size_t num_of_devices,
    std::size_t k) const
  {
    auto const              new_graph = block_graph(num_of_devices);
    type_collection_weights new_weight_map;

    auto new_weights_fun = block_graph_weights(new_weight_map,
                                               weights,
                                               transmission_weights,
                                               new_graph);

    KFinder_Lazy_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res = kFinder.lazy_eppstein(new_weights_fun, k);

    return {new_graph, res};
  }


  std::vector<std::pair<onnx::ModelProto, std::size_t>>
  reconstruct_model(std::pair<Graph<node_id_type, node_id_type>,
                              path_info> const &output_shortest)
  {
    return reconstruct_model(output_shortest.first, output_shortest.second);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  std::vector<std::pair<onnx::ModelProto, std::size_t>>
  reconstruct_model(Graph<node_id_type, node_id_type> const &new_graph,
                    path_info const                         &path)
  {
    auto const &original_model = graph.original_model;
    auto const &path_nodes     = path.path;
    std::vector<std::pair<onnx::ModelProto, std::size_t>> res;
    auto const new_graph_size = new_graph.nodes.size();

    std::unordered_map<node_id_type, std::set<node_id_type>>
      map; // New node -> Old nodes

    for (auto const &node : new_graph.nodes_content)
      map[node.second].insert(node.first);

    std::size_t current_model_device =
      path_nodes.front() < new_graph_size ?
        0 :
        (path_nodes.front() - 2) / (new_graph_size - 2);

    res.emplace_back(onnx::ModelProto(), current_model_device);

    std::function<void(onnx::ModelProto &)> prepare_new_model =
      [&original_model](onnx::ModelProto &new_model) {
        new_model.set_doc_string(original_model.doc_string());
        new_model.set_domain(original_model.domain());
        new_model.set_producer_name(original_model.producer_name());
        new_model.set_producer_version(original_model.producer_version());
      };
    std::function<onnx::GraphProto *()> prepare_new_graph =
      [&original_model]() {
        auto new_graph_pointer = new onnx::GraphProto;
        new_graph_pointer->set_name(original_model.graph().name());
        new_graph_pointer->set_doc_string(original_model.graph().doc_string());
        return new_graph_pointer;
      };

    prepare_new_model(res.back().first);

    auto        tmp_graph   = prepare_new_graph();
    auto const &model_graph = original_model.graph();

    std::function<google::protobuf::internal::RepeatedPtrIterator<
      const onnx::ValueInfoProto>(std::string const &)>
      get_type = [&](std::string const &communication_node_name) {
        auto tmp_res =
          std::find_if(original_model.graph().input().begin(),
                       original_model.graph().input().end(),
                       [communication_node_name](auto const &ref) {
                         return ref.name() == communication_node_name;
                       });

        if (tmp_res == original_model.graph().input().end())
          tmp_res = std::find_if(original_model.graph().output().begin(),
                                 original_model.graph().output().end(),
                                 [communication_node_name](auto const &ref) {
                                   return ref.name() == communication_node_name;
                                 });
        else
          return tmp_res;

        if (tmp_res == original_model.graph().output().end())
          tmp_res = std::find_if(original_model.graph().value_info().begin(),
                                 original_model.graph().value_info().end(),
                                 [communication_node_name](auto const &ref) {
                                   return ref.name() == communication_node_name;
                                 });

        return tmp_res;
      };


    std::function<void(onnx::GraphProto *, onnx::NodeProto const *)>
      add_to_graph = [&model_graph](onnx::GraphProto      *sup_graph,
                                    onnx::NodeProto const *node) {
        auto const &graph = model_graph;
        for (std::size_t i = 0; i < node->input_size(); ++i)
          {
            auto it =
              std::find_if(graph.input().begin(),
                           graph.input().end(),
                           [&node, &i](onnx::ValueInfoProto const &ref) {
                             return node->input(i) == ref.name();
                           });

            if (it != graph.input().end())
              {
                auto const tmp = sup_graph->add_input();
                *tmp           = *it;
              }
            else
              {
                it = std::find_if(graph.value_info().begin(),
                                  graph.value_info().end(),
                                  [&node, &i](onnx::ValueInfoProto const &ref) {
                                    return node->input(i) == ref.name();
                                  });

                if (it != graph.value_info().end())
                  {
                    auto const tmp = sup_graph->add_value_info();
                    *tmp           = *it;
                  }
              }

            auto init = std::find_if(graph.initializer().begin(),
                                     graph.initializer().end(),
                                     [&node, &i](onnx::TensorProto const &ref) {
                                       return node->input(i) == ref.name();
                                     });
            if (init != graph.initializer().end())
              {
                auto const tmp = sup_graph->add_initializer();
                *tmp           = *init;
              }
          }
      };


    std::function<void(std::size_t const &)> add_nodes =
      [&](std::size_t const &id) {
        for (auto const &original_id : map[id])
          {
            auto const tmp = tmp_graph->add_node();
            *tmp           = *graph.node_collection[original_id];

            add_to_graph(tmp_graph, tmp);
          }
      };

    for (std::size_t i = 0; i < path_nodes.size(); ++i)
      {
        auto const ind = path_nodes[i];
        auto const actual_id =
          ind < new_graph_size ? ind : (ind - 2) % (new_graph_size - 2) + 1;

        std::size_t device_id =
          ind < new_graph_size ? 0 : (ind - 2) / (new_graph_size - 2);

        if (device_id != current_model_device)
          {
            auto tmp_last_graph = tmp_graph;

            current_model_device = device_id;

            res.emplace_back(onnx::ModelProto(), current_model_device);
            prepare_new_model(res.back().first);

            auto tmp_new_graph = prepare_new_graph();

            auto const out_nodes      = map[actual_id - 1];
            auto const out_nodes_size = out_nodes.size();
            std::vector<onnx::NodeProto const *> communication_nodes;

            auto const &out =
              *graph.dependencies[*out_nodes.rbegin()].second.begin();

            for (auto set_it = out_nodes.rbegin(); set_it != out_nodes.rend();
                 ++set_it)
              if (graph.dependencies[*set_it].second.contains(out))
                communication_nodes.push_back(graph.node_collection[*set_it]);

            for (auto const &in_node : communication_nodes)
              {
                auto tmp_out = tmp_last_graph->add_output();
                auto tmp_in  = tmp_new_graph->add_input();

                auto communication_node_name = *in_node->output().begin();

                tmp_out->set_name(communication_node_name);
                tmp_in->set_name(communication_node_name);

                auto tmp_res = get_type(communication_node_name);

                if (tmp_res != original_model.graph().input().end())
                  {
                    tmp_out->set_allocated_type(
                      new onnx::TypeProto(tmp_res->type()));
                    tmp_in->set_allocated_type(
                      new onnx::TypeProto(tmp_res->type()));
                  }
              }

            (++res.rbegin())->first.set_allocated_graph(tmp_last_graph);
            tmp_graph = tmp_new_graph;

            add_nodes(actual_id);

            for (auto it = tmp_graph->mutable_node()->begin();
                 it != tmp_graph->mutable_node()->end();
                 ++it)
              {
                auto const &ins = it->input();
                for (auto const &in : ins)
                  {
                    bool ok = false;
                    for (std::size_t j = 0; j < tmp_graph->input_size() && !ok;
                         ++j)
                      if (tmp_graph->input(j).name() == in)
                        ok = true;

                    for (auto it2 = tmp_graph->mutable_node()->begin();
                         it2 != it && !ok;
                         ++it2)
                      {
                        for (std::size_t j = 0; j < it2->output_size() && !ok;
                             ++j)
                          if (it2->output(j) == in)
                            ok = true;
                      }
                    if (!ok)
                      {
                        auto tmp_in  = tmp_graph->add_input();
                        auto tmp_res = get_type(in);

                        tmp_in->set_name(in);
                        if (tmp_res != original_model.graph().input().end())
                          tmp_in->set_allocated_type(
                            new onnx::TypeProto(tmp_res->type()));
                      }
                  }
              }
          }
        else
          add_nodes(actual_id);
      }

    {
      auto tmp = tmp_graph->add_output();
      *tmp     = model_graph.output(model_graph.output_size() - 1);
    }

    res.back().first.set_allocated_graph(tmp_graph);

    return res;
  }
};


#endif // NETWORK_BUTCHER_BUTCHER_H
