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


  /// It will produce a linearized version of the current graph
  /// \return The linearized graph
  [[nodiscard]] Graph<node_id_type, node_id_type>
  block_graph() const
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

    new_dependencies.reserve(new_nodes.size());
    new_dependencies.emplace_back(
      std::make_pair<node_id_collection_type, node_id_collection_type>({},
                                                                       {1}));

    // Since the graph is linear, the input of the i-th node is the node i-1
    // while the output is the node i+1
    for (node_id_type i = 1; i < new_nodes.size() - 1; ++i)
      new_dependencies.emplace_back(
        std::make_pair<node_id_collection_type, node_id_collection_type>(
          {i - 1}, {i + 1}));

    new_dependencies.emplace_back(
      std::make_pair<node_id_collection_type, node_id_collection_type>(
        {new_nodes.size() - 2}, {}));

    return Graph(new_nodes, new_content, new_dependencies, false);
  }


  /// Given the current graph and the original weight function, it will produce
  /// a new weight function for the linearized graph
  /// \param new_weight_map The new weight map will look for the value of the
  /// weight firstly in this map. If it cannot find it, it will store the proper
  /// weight into the map (to make everything faster)
  /// \param original_weights The weight function for the original graph
  /// \param transmission_weights Used when we are switching from a device to
  /// another
  /// \param new_graph The lineatized graph (result of block_graph)
  /// \return The new weight function
  [[nodiscard]] std::function<type_weight(edge_type const &)>
  block_graph_weights(
    type_collection_weights                       &new_weight_map,
    std::function<type_weight(edge_type const &)> &original_weights,
    std::function<type_weight(edge_type const &)> &transmission_weights,
    Graph<node_id_type, node_id_type> const       &new_graph) const
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

      auto const size = new_graph.nodes.size();

      // The index of the input node on the linearized graph
      auto const first_index =
        edge.first < size ? edge.first : (edge.first - 2) % (size - 2) + 1;

      // The index of the output node on the linearized graph
      auto const second_index =
        edge.second < size ? edge.second : (edge.second - 2) % (size - 2) + 1;


      if (first_index > second_index && second_index + 1 != first_index ||
          second_index > first_index && first_index + 1 != second_index ||
          first_index == second_index)
        {
          std::cout
            << "Requested an invalid edge: check if the graph is correct!"
            << std::endl;
          return -1.;
        }

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

      // The device id of the input node (=0 starting device, >0 other device)
      auto const in_device_id =
        edge.first < size ? 0 : (edge.first - 2) / (size - 2);
      // The device id of the output node (=0 starting device, >0 other device)
      auto const out_device_id =
        edge.second < size ? 0 : (edge.second - 2) / (size - 2);

      // The padding to be inserted to the nodes of the original graph to obtain
      // the corresponding node on the multi-device graph
      auto const in_index_adj =
        in_device_id == 0 ? 0 : 1 + (graph.nodes.size() - 2) * in_device_id;
      auto const out_index_adj =
        out_device_id == 0 ? 0 : 1 + (graph.nodes.size() - 2) * out_device_id;

      // 1-1 correspondence
      if (outputs.size() == 1 && inputs.size() == 1)
        {
          if (first_index == 0)
            {
              auto const head    = *outputs.begin() + out_index_adj;
              auto const tm_edge = std::make_pair(*inputs.begin(), head);

              auto const res =
                original_weights(tm_edge) + transmission_weights(tm_edge);

              new_weight_map[edge] = res;
              return res;
            }
          else if (second_index == size - 1)
            {
              auto const tail    = *inputs.begin() + in_index_adj;
              auto const tm_edge = std::make_pair(tail, *outputs.begin());

              auto const res =
                original_weights(tm_edge) + transmission_weights(tm_edge);

              new_weight_map[edge] = res;
              return res;
            }
          else
            {
              auto const tail_in  = *inputs.begin() + in_index_adj;
              auto const tail_out = *inputs.begin() + out_index_adj;
              auto const head     = *outputs.begin() + out_index_adj;

              auto const res = original_weights({tail_out, head}) +
                               transmission_weights({tail_in, head});

              new_weight_map[edge] = res;
              return res;
            }
        }
      // (2+)-1 correspondence. The idea is that the input nodes must transmit
      // to the output node the different values. Thus, the transmission cost is
      // paid serveral times
      else if (outputs.size() == 1)
        {
          type_weight res = .0;
          // For convention, the last node is a "communication" node. Thus, we
          // read the weights directly from the real input id
          if (second_index == size - 1)
            {
              for (auto const &exit :
                   graph.dependencies[*outputs.begin()].first)
                {
                  auto const weight =
                    original_weights({exit + in_index_adj, *outputs.begin()}) +
                    transmission_weights(
                      {exit + in_index_adj, *outputs.begin()});

                  if (weight < 0)
                    return -1.;

                  res += weight;
                }
            }
          // Here, the weight is read on the appropriate nodes of the original
          // graph
          else
            {
              for (auto const &exit :
                   graph.dependencies[*outputs.begin()].first)
                {
                  auto const weight =
                    original_weights({exit + out_index_adj,
                                      *outputs.begin() + out_index_adj}) +
                    transmission_weights(
                      {exit + in_index_adj, *outputs.begin() + out_index_adj});

                  if (weight < 0)
                    return -1.;

                  res += weight;
                }
            }
          new_weight_map[edge] = res;
          return res;
        }
      // 1-(2+). In this case, the input is sent to the device of the output
      // nodes a single time. Thus, this transmission cost is taken into account
      // only once.
      else if (inputs.size() == 1)
        {
          type_weight res = .0;

          // If it's the starting node, then, by convention, the weights are
          // explicit
          if (first_index == 0)
            {
              for (auto const &node :
                   graph.dependencies[*inputs.begin()].second)
                {
                  auto const weight =
                    original_weights({*inputs.begin(), node + out_index_adj});

                  if (weight < 0)
                    return -1.;

                  res += weight;
                }

              res += transmission_weights(
                {*inputs.begin(),
                 *graph.dependencies[*inputs.begin()].second.begin() +
                   out_index_adj});
            }
          // In this case, the weights are implicit. Thus, we have to look for
          // the correct weights
          else
            {
              for (auto const &node :
                   graph.dependencies[*inputs.begin()].second)
                {
                  auto const weight = original_weights(
                    {*inputs.begin() + out_index_adj, node + out_index_adj});

                  if (weight < 0)
                    return -1.;

                  res += weight;
                }

              res += transmission_weights(
                {*inputs.begin() + in_index_adj,
                 *graph.dependencies[*inputs.begin()].second.begin() +
                   out_index_adj});
            }

          // Compute the total weight associated to the non-outputs node of the
          // output node.
          for (auto const &node : outputs)
            for (auto &dep : graph.dependencies[node].second)
              if (outputs.find(dep) != outputs.cend())
                {
                  auto const weight = original_weights(
                    {node + out_index_adj, dep + out_index_adj});

                  if (weight < 0)
                    return -1.;

                  res += weight;
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
  /// \param weights The weight map function, that associates to every edge
  /// (also the "fake" ones) the corresponding weight
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The k shortest paths
  std::vector<
    typename Shortest_path_finder<node_id_type, node_id_type>::path_info>
  compute_k_shortest_paths_eppstein_linear(
    std::function<type_weight(edge_type const &)> &weights,
    std::function<type_weight(edge_type const &)> &transmission_weights,
    std::size_t                                    num_of_devices,
    std::size_t                                    k) const
  {
    auto const              new_graph = block_graph();
    type_collection_weights new_weight_map;

    auto new_weights_fun = block_graph_weights(new_weight_map,
                                               weights,
                                               transmission_weights,
                                               new_graph);

    KFinder_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res =
      kFinder.eppstein_linear(new_weights_fun, k, num_of_devices);

    return res;
  }


  /// It will prodice the k-shortest paths for the linearized block graph
  /// associated with the original one
  /// \param weights The weight map function, that associates to every edge
  /// (also the "fake" ones) the corresponding weight
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The k shortest paths
  std::vector<
    typename Shortest_path_finder<node_id_type, node_id_type>::path_info>
  compute_k_shortest_paths_lazy_eppstein_linear(
    std::function<type_weight(edge_type const &)> &weights,
    std::function<type_weight(edge_type const &)> &transmission_weights,
    std::size_t                                    num_of_devices,
    std::size_t                                    k) const
  {
    auto const              new_graph = block_graph();
    type_collection_weights new_weight_map;

    auto new_weights_fun = block_graph_weights(new_weight_map,
                                               weights,
                                               transmission_weights,
                                               new_graph);

    KFinder_Lazy_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res =
      kFinder.lazy_eppstein_linear(new_weights_fun, k, num_of_devices);

    return res;
  }
};


/// Butcher butchers a given graph into slices
template <>
class Butcher<graph_input_type>
{
private:
  using network = Graph<graph_input_type>;

  onnx::ModelProto original_model;
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


  /// It will produce a linearized version of the current graph
  /// \return The linearized graph
  [[nodiscard]] Graph<node_id_type, node_id_type>
  block_graph() const
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

    new_dependencies.reserve(new_nodes.size());
    new_dependencies.emplace_back(
      std::make_pair<node_id_collection_type, node_id_collection_type>({},
                                                                       {1}));

    // Since the graph is linear, the input of the i-th node is the node i-1
    // while the output is the node i+1
    for (node_id_type i = 1; i < new_nodes.size() - 1; ++i)
      new_dependencies.emplace_back(
        std::make_pair<node_id_collection_type, node_id_collection_type>(
          {i - 1}, {i + 1}));

    new_dependencies.emplace_back(
      std::make_pair<node_id_collection_type, node_id_collection_type>(
        {new_nodes.size() - 2}, {}));

    return Graph(new_nodes, new_content, new_dependencies, false);
  }


  /// Given the current graph and the original weight function, it will produce
  /// a new weight function for the linearized graph
  /// \param new_weight_map The new weight map will look for the value of the
  /// weight firstly in this map. If it cannot find it, it will store the proper
  /// weight into the map (to make everything faster)
  /// \param original_weights The weight function for the original graph
  /// \param transmission_weights Used when we are switching from a device to
  /// another
  /// \param new_graph The lineatized graph (result of block_graph)
  /// \return The new weight function
  [[nodiscard]] std::function<type_weight(edge_type const &)>
  block_graph_weights(
    type_collection_weights                       &new_weight_map,
    std::function<type_weight(edge_type const &)> &original_weights,
    std::function<type_weight(edge_type const &)> &transmission_weights,
    Graph<node_id_type, node_id_type> const       &new_graph) const
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

      auto const size = new_graph.nodes.size();

      // The index of the input node on the linearized graph
      auto const first_index =
        edge.first < size ? edge.first : (edge.first - 2) % (size - 2) + 1;

      // The index of the output node on the linearized graph
      auto const second_index =
        edge.second < size ? edge.second : (edge.second - 2) % (size - 2) + 1;


      if (first_index > second_index && second_index + 1 != first_index ||
          second_index > first_index && first_index + 1 != second_index ||
          first_index == second_index)
        {
          std::cout
            << "Requested an invalid edge: check if the graph is correct!"
            << std::endl;
          return -1.;
        }

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

      // The device id of the input node (=0 starting device, >0 other device)
      auto const in_device_id =
        edge.first < size ? 0 : (edge.first - 2) / (size - 2);
      // The device id of the output node (=0 starting device, >0 other device)
      auto const out_device_id =
        edge.second < size ? 0 : (edge.second - 2) / (size - 2);

      // The padding to be inserted to the nodes of the original graph to obtain
      // the corresponding node on the multi-device graph
      auto const in_index_adj =
        in_device_id == 0 ? 0 : 1 + (graph.nodes.size() - 2) * in_device_id;
      auto const out_index_adj =
        out_device_id == 0 ? 0 : 1 + (graph.nodes.size() - 2) * out_device_id;

      // 1-1 correspondence
      if (outputs.size() == 1 && inputs.size() == 1)
        {
          if (first_index == 0)
            {
              auto const head    = *outputs.begin() + out_index_adj;
              auto const tm_edge = std::make_pair(*inputs.begin(), head);

              auto const res =
                original_weights(tm_edge) + transmission_weights(tm_edge);

              new_weight_map[edge] = res;
              return res;
            }
          else if (second_index == size - 1)
            {
              auto const tail    = *inputs.begin() + in_index_adj;
              auto const tm_edge = std::make_pair(tail, *outputs.begin());

              auto const res =
                original_weights(tm_edge) + transmission_weights(tm_edge);

              new_weight_map[edge] = res;
              return res;
            }
          else
            {
              auto const tail_in  = *inputs.begin() + in_index_adj;
              auto const tail_out = *inputs.begin() + out_index_adj;
              auto const head     = *outputs.begin() + out_index_adj;

              auto const res = original_weights({tail_out, head}) +
                               transmission_weights({tail_in, head});

              new_weight_map[edge] = res;
              return res;
            }
        }
      // (2+)-1 correspondence. The idea is that the input nodes must transmit
      // to the output node the different values. Thus, the transmission cost is
      // paid serveral times
      else if (outputs.size() == 1)
        {
          type_weight res = .0;
          // For convention, the last node is a "communication" node. Thus, we
          // read the weights directly from the real input id
          if (second_index == size - 1)
            {
              for (auto const &exit :
                   graph.dependencies[*outputs.begin()].first)
                {
                  auto const weight =
                    original_weights({exit + in_index_adj, *outputs.begin()}) +
                    transmission_weights(
                      {exit + in_index_adj, *outputs.begin()});

                  if (weight < 0)
                    return -1.;

                  res += weight;
                }
            }
          // Here, the weight is read on the appropriate nodes of the original
          // graph
          else
            {
              for (auto const &exit :
                   graph.dependencies[*outputs.begin()].first)
                {
                  auto const weight =
                    original_weights({exit + out_index_adj,
                                      *outputs.begin() + out_index_adj}) +
                    transmission_weights(
                      {exit + in_index_adj, *outputs.begin() + out_index_adj});

                  if (weight < 0)
                    return -1.;

                  res += weight;
                }
            }
          new_weight_map[edge] = res;
          return res;
        }
      // 1-(2+). In this case, the input is sent to the device of the output
      // nodes a single time. Thus, this transmission cost is taken into account
      // only once.
      else if (inputs.size() == 1)
        {
          type_weight res = .0;

          // If it's the starting node, then, by convention, the weights are
          // explicit
          if (first_index == 0)
            {
              for (auto const &node :
                   graph.dependencies[*inputs.begin()].second)
                {
                  auto const weight =
                    original_weights({*inputs.begin(), node + out_index_adj});

                  if (weight < 0)
                    return -1.;

                  res += weight;
                }

              res += transmission_weights(
                {*inputs.begin(),
                 *graph.dependencies[*inputs.begin()].second.begin() +
                   out_index_adj});
            }
          // In this case, the weights are implicit. Thus, we have to look for
          // the correct weights
          else
            {
              for (auto const &node :
                   graph.dependencies[*inputs.begin()].second)
                {
                  auto const weight = original_weights(
                    {*inputs.begin() + out_index_adj, node + out_index_adj});

                  if (weight < 0)
                    return -1.;

                  res += weight;
                }

              res += transmission_weights(
                {*inputs.begin() + in_index_adj,
                 *graph.dependencies[*inputs.begin()].second.begin() +
                   out_index_adj});
            }

          // Compute the total weight associated to the non-outputs node of the
          // output node.
          for (auto const &node : outputs)
            for (auto &dep : graph.dependencies[node].second)
              if (outputs.find(dep) != outputs.cend())
                {
                  auto const weight = original_weights(
                    {node + out_index_adj, dep + out_index_adj});

                  if (weight < 0)
                    return -1.;

                  res += weight;
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
    : original_model(utilities::parse_onnx_file(p))
    , graph(original_model, ignore_parameters){};


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
  /// \param weights The weight map function, that associates to every edge
  /// (also the "fake" ones) the corresponding weight
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The auxiliary graph and the k-shortest paths on the auxiliary
  /// graph
  std::pair<Graph<node_id_type, node_id_type>,
            std::vector<typename Shortest_path_finder<node_id_type,
                                                      node_id_type>::path_info>>
  compute_k_shortest_paths_eppstein_linear(
    std::function<type_weight(edge_type const &)> &weights,
    std::function<type_weight(edge_type const &)> &transmission_weights,
    std::size_t                                    num_of_devices,
    std::size_t                                    k) const
  {
    auto const              new_graph = block_graph();
    type_collection_weights new_weight_map;

    auto new_weights_fun = block_graph_weights(new_weight_map,
                                               weights,
                                               transmission_weights,
                                               new_graph);

    KFinder_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res =
      kFinder.eppstein_linear(new_weights_fun, k, num_of_devices);

    return {new_graph, res};
  }


  /// It will prodice the k-shortest paths for the linearized block graph
  /// associated with the original one
  /// \param weights The weight map function, that associates to every edge
  /// (also the "fake" ones) the corresponding weight
  /// \param num_of_devices The number of devices
  /// \param k The number of shortest paths to find
  /// \return The auxiliary graph and the k-shortest paths on the auxiliary
  /// graph
  std::pair<Graph<node_id_type, node_id_type>,
            std::vector<typename Shortest_path_finder<node_id_type,
                                                      node_id_type>::path_info>>
  compute_k_shortest_paths_lazy_eppstein_linear(
    std::function<type_weight(edge_type const &)> &weights,
    std::function<type_weight(edge_type const &)> &transmission_weights,
    std::size_t                                    num_of_devices,
    std::size_t                                    k) const
  {
    auto const              new_graph = block_graph();
    type_collection_weights new_weight_map;

    auto new_weights_fun = block_graph_weights(new_weight_map,
                                               weights,
                                               transmission_weights,
                                               new_graph);

    KFinder_Lazy_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res =
      kFinder.lazy_eppstein_linear(new_weights_fun, k, num_of_devices);

    return {new_graph, res};
  }

  std::vector<std::pair<onnx::ModelProto, std::size_t>>
  reconstruct_model(
    std::pair<Graph<node_id_type, node_id_type>,
              Shortest_path_finder<node_id_type, node_id_type>::path_info> const
      &output_shortest)
  {
    return reconstruct_model(output_shortest.first, output_shortest.second);
  }

  std::vector<std::pair<onnx::ModelProto, std::size_t>>
  reconstruct_model(
    Graph<node_id_type, node_id_type> const &new_graph,
    Shortest_path_finder<node_id_type, node_id_type>::path_info const &path)
  {
    auto const                   &path_nodes = path.path;
    std::vector<onnx::ModelProto> res;
    auto const                    new_graph_size = new_graph.nodes.size();

    std::size_t current_model_device =
      path_nodes.front() < new_graph_size ?
        0 :
        (path_nodes.front() - 2) / (new_graph_size - 2);

    onnx::ModelProto partial_res = original_model;
    std::size_t      start_node  = 0;

    for (std::size_t i = 0; i < path.path.size(); ++i)
      {
        auto const &nodes  = *partial_res.graph().node().begin();
        auto const &output = *partial_res.graph().output().begin();
        auto const &input  = *partial_res.graph().input().begin();

        auto const name = nodes.name();

        std::cout << std::endl;
      }

    return {{partial_res, 0}};
  }
};


#endif // NETWORK_BUTCHER_BUTCHER_H
