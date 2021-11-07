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
  /// Note that, at the end, all the ok slices will be moved to the return
  /// vector, while the non-compatible ones will be deleted
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
          res.push_back(std::move(slices[i]));
        else
          {
            {
              slice_type tmp(std::move(slices[i]));
            }
          }
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

    int counter = graph.dependencies.front().second.size() -
                  graph.dependencies.front().first.size() - 1;
    std::size_t id = 0;

    new_nodes.reserve(graph.nodes.size());
    new_nodes.emplace_back(id++);
    new_content[0] = 0;

    for (auto it = ++graph.nodes.begin(); it != graph.nodes.end(); ++it)
      {
        auto const &node          = *it;
        auto const &dep           = graph.dependencies[node.get_id()];
        int const   local_counter = dep.second.size() - dep.first.size();

        // Add new node
        if (local_counter <= 0 && counter == 0)
          {
            new_nodes.emplace_back(id);
            new_content[node.get_id()] = id;

            ++id;
          }
        // Add new node and add master node for next steps
        else if (local_counter > 0 && counter == 0)
          {
            new_nodes.emplace_back(id);
            new_content[node.get_id()] = id;

            ++id;

            new_nodes.emplace_back(id);
            new_content[id];

            counter += local_counter;
          }
        // Add node link to the "big" node
        else if (local_counter == 0 && dep.second.size() == 1 && counter > 0)
          {
            new_content[node.get_id()] = id;
          }
        else if (local_counter > 0 && counter > 0 && dep.first.size() <= 1)
          {
            new_content[node.get_id()] = id;
            counter += local_counter;
          }
        else if (counter > 0 && ((local_counter >= 0 && dep.first.size() > 1) ||
                                 (local_counter < 0)))
          {
            counter -= (dep.first.size() - 1);
            if (counter == 0)
              {
                new_nodes.emplace_back(++id);
                new_content[node.get_id()] = id;

                if (local_counter >= 0)
                  {
                    new_nodes.emplace_back(++id);
                    new_content[id];
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
      auto const candidate_sol = new_weight_map.find(edge);
      if (candidate_sol != new_weight_map.cend())
        return candidate_sol->second;

      auto const size = new_graph.nodes.size();

      auto const first_index =
        edge.first < size ? edge.first : (edge.first - 2) % (size - 2) + 1;

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

      auto const it_out = map.find(second_index);
      if (it_out == map.cend() || it_out->second.size() == 0)
        return -1.;

      auto const it_in = map.find(first_index);
      if (it_in == map.cend() || it_in->second.size() == 0)
        return -1.;

      auto const &inputs  = it_in->second;
      auto const &outputs = it_out->second;

      auto const in_device_id =
        edge.first < size ? 0 : (edge.first - 2) / (size - 2);
      auto const out_device_id =
        edge.second < size ? 0 : (edge.second - 2) / (size - 2);

      auto const in_index_adj =
        in_device_id == 0 ? 0 : 1 + (graph.nodes.size() - 2) * in_device_id;
      auto const out_index_adj =
        out_device_id == 0 ? 0 : 1 + (graph.nodes.size() - 2) * out_device_id;

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
      else if (outputs.size() == 1)
        {
          type_weight res = .0;
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
      else if (inputs.size() == 1)
        {
          type_weight res = .0;

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
            }
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
  Butcher(const std::string &p, bool ignore_parameters = true)
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
  std::vector<typename KFinder<node_id_type, node_id_type>::path_info>
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
  std::vector<typename KFinder<node_id_type, node_id_type>::path_info>
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

    KFinder_Eppstein<node_id_type, node_id_type> kFinder(new_graph);

    auto const res =
      kFinder.lazy_eppstein_linear(new_weights_fun, k, num_of_devices);

    return res;
  }
};


#endif // NETWORK_BUTCHER_BUTCHER_H
