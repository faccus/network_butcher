//
// Created by faccus on 26/10/21.
//

#ifndef NETWORK_BUTCHER_KSP_H
#define NETWORK_BUTCHER_KSP_H

#include "../Traits/Graph_traits.h"

#include "Heap_eppstein.h"

#include <limits>
#include <queue>
#include <vector>


template <class T, typename id_content = io_id_type>
class KFinder
{
public:
  struct path_info
  {
    type_weight               length;
    std::vector<node_id_type> path;

    constexpr bool
    operator<(const path_info &rhs) const
    {
      return length < rhs.length || (length == rhs.length && path < rhs.path);
    }
  };

  explicit KFinder(Graph<T, id_content> const &g)
    : graph(g){};


  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  dijkstra(type_collection_weights const &weights,
           node_id_type                   root = 0,
           bool reversed = false) const // time: ((N+E)log(N)), space: O(N)
  {
    std::function<type_weight(edge_type const &)> weight_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it != weights.cend())
          return it->second;
        return -1.;
      };

    return dijkstra(weight_fun, root, reversed);
  }

  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  dijkstra(std::function<type_weight(edge_type const &)> &weights,
           node_id_type                                   root = 0,
           bool reversed = false) const // time: ((N+E)log(N)), space: O(N)
  {
    if (graph.nodes.empty())
      return {{}, {}};

    std::vector<type_weight> total_distance(graph.nodes.size(),
                                            std::numeric_limits<double>::max());
    total_distance[root] = 0;

    std::vector<node_id_type>        predecessors(graph.nodes.size(), root);
    std::set<dijkstra_helper_struct> to_visit{{0, root}};
    auto const                      &dependencies = graph.dependencies;

    while (!to_visit.empty()) // O(N)
      {
        auto current_node = *to_visit.begin(); // O(1)

        auto const &start_distance = total_distance[current_node.id];
        if (start_distance == std::numeric_limits<type_weight>::max())
          {
            std::cout << "Error" << std::endl;
            return {predecessors, total_distance};
          }

        to_visit.erase(to_visit.begin()); // O(log(N))

        auto const &exit_nodes = reversed ?
                                   dependencies[current_node.id].first :
                                   dependencies[current_node.id].second; // O(1)

        for (auto j : exit_nodes)
          {
            auto      &basic_dist = total_distance[j]; // O(1)
            auto const ref        = reversed ? weights({j, current_node.id}) :
                                               weights({current_node.id, j});

            if (ref < 0)
              {
                if (!reversed)
                  std::cout << "Error: missing weight (" << current_node.id
                            << ", " << j << ")" << std::endl;
                else
                  std::cout << "Error: missing weight (" << j << ", "
                            << current_node.id << ")" << std::endl;
                return {predecessors, total_distance};
              }

            auto const candidate_distance = start_distance + ref; // O(1)
            if (candidate_distance < basic_dist)                  // O(1)
              {
                auto it = to_visit.find({basic_dist, j});

                if (it != to_visit.end())
                  to_visit.erase(it); // O(log(N))

                predecessors[j] = current_node.id;        // O(1)
                basic_dist      = candidate_distance;     // O(1)
                to_visit.insert({candidate_distance, j}); // O(log(N))
              }
          }
      }

    return {predecessors, total_distance};
  }


  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  dijkstra_linear(type_collection_weights const &weights,
                  node_id_type                   root,
                  bool                           reversed,
                  std::size_t                    devices)
    const // time: (devices * (N+E)log(devices * N)), space: O(N)
  {
    std::function<type_weight(edge_type const &)> weight_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it != weights.cend())
          return it->second;
        return -1.;
      };

    return dijkstra_linear(weights, root, reversed, devices);
  }

  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  dijkstra_linear(std::function<type_weight(edge_type const &)> &weights,
                  node_id_type                                   root,
                  bool                                           reversed,
                  std::size_t                                    devices)
    const // time: (devices * (N+E)log(devices * N)), space: O(N)
  {
    if (devices == 1)
      return dijkstra(weights, root, reversed);

    if (graph.nodes.empty() || devices == 0)
      return {{}, {}};


    std::vector<type_weight> total_distance(graph.nodes.size() +
                                              (graph.nodes.size() - 2) *
                                                (devices - 1),
                                            std::numeric_limits<double>::max());
    total_distance[root] = 0;

    std::vector<node_id_type> predecessors(
      graph.nodes.size() + (graph.nodes.size() - 2) * (devices - 1), root);
    std::set<dijkstra_helper_struct> to_visit{{0, root}};

    auto       dependencies = graph.dependencies;
    auto const num_nodes    = graph.nodes.size();

    while (!to_visit.empty()) // O(N)
      {
        auto current_node = *to_visit.begin(); // O(1)

        auto const &start_distance = total_distance[current_node.id];
        if (start_distance == std::numeric_limits<type_weight>::max())
          {
            std::cout << "Error" << std::endl;
            return {predecessors, total_distance};
          }

        to_visit.erase(to_visit.begin()); // O(1)

        auto const tm_exit_nodes =
          current_node.id < num_nodes ?
            current_node.id :
            (current_node.id - 2) % (num_nodes - 2) + 1;

        auto exit_nodes = reversed ? dependencies[tm_exit_nodes].first :
                                     dependencies[tm_exit_nodes].second;

        for (auto j : exit_nodes) // O(E)
          {
            auto func = [&](node_id_type const &head) {
              auto tail = current_node.id;

              auto      &basic_dist = total_distance[head]; // O(1)
              auto const ref =
                reversed ? weights({head, tail}) : weights({tail, head});

              if (ref < 0)
                {
                  if (!reversed)
                    std::cout << "Error: missing weight (" << head << ", "
                              << tail << ")" << std::endl;
                  else
                    std::cout << "Error: missing weight (" << tail << ", "
                              << head << ")" << std::endl;
                }
              else
                {
                  auto const candidate_distance = start_distance + ref; // O(1)
                  if (candidate_distance < basic_dist)                  // O(1)
                    {
                      auto it = to_visit.find({basic_dist, head});

                      if (it != to_visit.end())
                        to_visit.erase(it); // O(log(N))

                      predecessors[head] = current_node.id;        // O(1)
                      basic_dist         = candidate_distance;     // O(1)
                      to_visit.insert({candidate_distance, head}); // O(log(N))
                    }
                }
            };
            func(j);

            if (!(reversed && j == 0) && !(!reversed && j == num_nodes - 1))
              {
                for (std::size_t k = 1; k < devices; ++k)
                  func(j + num_nodes - 1 + (k - 1) * (num_nodes - 2));
              }
          }
      }

    return {predecessors, total_distance};
  }

  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  shortest_path_tree(type_collection_weights const &weights) const
  {
    return dijkstra(weights, graph.nodes.size() - 1, true);
  } // time: ((N+E)log(N)), space: O(N)

  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  shortest_path_tree_linear(type_collection_weights const &weights,
                            std::size_t                    devices) const
  {
    return dijkstra_linear(weights, graph.nodes.size() - 1, true, devices);
  } // time: ((N+E)log(N)), space: O(N)

  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  shortest_path_tree(
    std::function<type_weight(edge_type const &)> &weights) const
  {
    return dijkstra(weights, graph.nodes.size() - 1, true);
  } // time: ((N+E)log(N)), space: O(N)

  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  shortest_path_tree_linear(
    std::function<type_weight(edge_type const &)> &weights,
    std::size_t                                    devices) const
  {
    return dijkstra_linear(weights, graph.nodes.size() - 1, true, devices);
  } // time: ((N+E)log(N)), space: O(N)


protected:
  struct dijkstra_helper_struct
  {
    type_weight  weight;
    node_id_type id;

    constexpr bool
    operator<(const dijkstra_helper_struct &rhs) const
    {
      return weight < rhs.weight || (weight == rhs.weight && id < rhs.id);
    }
  };

  Graph<T, id_content> const &graph;


  path_info
  shortest_path_finder(std::pair<std::vector<node_id_type>,
                                 std::vector<type_weight>> const &dij_res,
                       node_id_type                               root) const
  {
    path_info info;
    info.length = dij_res.second[root];
    info.path.reserve(graph.nodes.size());

    auto ind = root;
    while (ind != graph.nodes.back().get_id())
      {
        info.path.push_back(ind);
        ind = dij_res.first[ind];
      }
    info.path.push_back(ind);

    return info;
  }
};


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
