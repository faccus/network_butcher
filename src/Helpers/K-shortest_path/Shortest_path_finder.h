//
// Created by faccus on 26/10/21.
//

#ifndef NETWORK_BUTCHER_SHORTEST_PATH_FINDER_H
#define NETWORK_BUTCHER_SHORTEST_PATH_FINDER_H

#include "../Traits/Graph_traits.h"

#include "Heap_eppstein.h"
#include "Path_info.h"

#include <limits>
#include <queue>
#include <vector>


template <class T, typename id_content = io_id_type>
class Shortest_path_finder
{
public:

  explicit Shortest_path_finder(Graph<T, id_content> const &g)
    : graph(g){};


  /// Executes dijkstra algorithm to compute the shortest paths from the root to
  /// evert node for the given graph \param weights The weight map of the edges
  /// \param root The starting vertex
  /// \param reversed Reverses the edge directions
  /// \return A pair: the first element is the collection of the successors
  /// (along the shortest path) of the different nodes while the second element
  /// is the shortest path length
  [[nodiscard]] dijkstra_result_type
  dijkstra(type_collection_weights const &weights,
           node_id_type                   root = 0,
           bool reversed = false) const // time: ((N+E)log(N)), space: O(N)
  {
    std::function<type_weight(edge_type const &)> weight_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it != weights.cend())
          return it->second;
        std::cout << "Dijkstra: missing weight" << std::endl;
        return -1.;
      };

    return dijkstra(weight_fun, root, reversed);
  }

  /// Executes dijkstra algorithm to compute the shortest paths from the root to
  /// evert node for the given graph \param weights The weight map of the edges
  /// \param root The starting vertex
  /// \param reversed Reverses the edge directions
  /// \return A pair: the first element is the collection of the successors
  /// (along the shortest path) of the different nodes while the second element
  /// is the shortest path length
  [[nodiscard]] dijkstra_result_type
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
            std::cout << "Dijkstra error: the current distance is +inf"
                      << std::endl;
            return {predecessors, total_distance};
          }

        to_visit.erase(to_visit.begin()); // O(log(N))

        for (auto head_node : extract_children(current_node.id, reversed))
          {
            auto      &base_distance = total_distance[head_node]; // O(1)
            auto const weight =
              get_weight(current_node.id, head_node, weights, reversed);

            if (weight < 0)
              {
                if (!reversed)
                  std::cout << "Error: missing weight (" << current_node.id
                            << ", " << head_node << ")" << std::endl;
                else
                  std::cout << "Error: missing weight (" << head_node << ", "
                            << current_node.id << ")" << std::endl;
                return {predecessors, total_distance};
              }

            auto const candidate_distance = start_distance + weight; // O(1)
            if (candidate_distance < base_distance)                  // O(1)
              {
                auto it = to_visit.find({base_distance, head_node});

                if (it != to_visit.end())
                  to_visit.erase(it); // O(log(N))

                predecessors[head_node] = current_node.id;        // O(1)
                base_distance           = candidate_distance;     // O(1)
                to_visit.insert({candidate_distance, head_node}); // O(log(N))
              }
          }
      }

    return {predecessors, total_distance};
  }


  /// (No longer maintained) Executes dijkstra algorithm to compute the shortest
  /// paths from the root to evert node for the given linear graph
  /// \param weights The weight map of the edges
  /// \param root The starting vertex
  /// \param reversed Reverses the edge directions
  /// \param devices The number of devices
  /// \return A pair: the first element is the collection of the successors
  /// (along the shortest path) of the different nodes while the second element
  /// is the shortest path length
  [[nodiscard]] dijkstra_result_type
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

  /// (No longer maintained) Executes dijkstra algorithm to compute the shortest
  /// paths from the root to evert node for the given linear graph
  /// \param weights The weight map of the edges
  /// \param root The starting vertex
  /// \param reversed Reverses the edge directions
  /// \param devices The number of devices
  /// \return A pair: the first element is the collection of the successors
  /// (along the shortest path) of the different nodes while the second element
  /// is the shortest path length
  [[nodiscard]] dijkstra_result_type
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
            std::cout << "Dijkstra error: the current distance is +inf"
                      << std::endl;
            return {predecessors, total_distance};
          }

        to_visit.erase(to_visit.begin()); // O(1)

        auto const tm_exit_nodes =
          current_node.id < num_nodes ?
            current_node.id :
            (current_node.id - 2) % (num_nodes - 2) + 1;

        auto exit_nodes = reversed ? dependencies[tm_exit_nodes].first :
                                     dependencies[tm_exit_nodes].second;

        for (auto j : extract_children(tm_exit_nodes, reversed))
          {
            auto func = [&](node_id_type const &head) {
              auto tail = current_node.id;

              auto      &basic_dist = total_distance[head]; // O(1)
              auto const weight     = get_weight(tail, head, weights, reversed);

              if (weight < 0)
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
                  auto const candidate_distance =
                    start_distance + weight;           // O(1)
                  if (candidate_distance < basic_dist) // O(1)
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

  /// Computes through dijkstra the shortest path single destination tree for
  /// the given graph
  /// \param weights The weight map of the edges
  /// \return A pair: the first element is the collection of the successors
  /// (along the shortest path) of the different nodes while the second element
  /// is the shortest path length
  [[nodiscard]] dijkstra_result_type
  shortest_path_tree(type_collection_weights const &weights) const
  {
    return dijkstra(weights, graph.nodes.size() - 1, true);
  } // time: ((N+E)log(N)), space: O(N)

  /// (No longer maintained) Computes through dijkstra the shortest path single
  /// destination tree for the given linear graph
  /// \param weights The weight map of the edges
  /// \param devices The number of devices
  /// \return A pair: the first element is the collection of the successors
  /// (along the shortest path) of the different nodes while the second element
  /// is the shortest path length
  [[nodiscard]] dijkstra_result_type
  shortest_path_tree_linear(type_collection_weights const &weights,
                            std::size_t                    devices) const
  {
    return dijkstra_linear(weights, graph.nodes.size() - 1, true, devices);
  } // time: ((N+E)log(N)), space: O(N)

  /// Computes through dijkstra the shortest path single destination tree for
  /// the given graph
  /// \param weights The weight map of the edges
  /// \return A pair: the first element is the collection of the successors
  /// (along the shortest path) of the different nodes while the second element
  /// is the shortest path length
  [[nodiscard]] dijkstra_result_type
  shortest_path_tree(
    std::function<type_weight(edge_type const &)> &weights) const
  {
    return dijkstra(weights, graph.nodes.size() - 1, true);
  } // time: ((N+E)log(N)), space: O(N)

  /// (No longer maintained) Computes through dijkstra the shortest path single
  /// destination tree for the given linear graph/// \param weights The weight
  /// map of the edges \param devices The number of devices \return A pair: the
  /// first element is the collection of the successors (along the shortest
  /// path) of the different nodes while the second element is the shortest path
  /// length
  [[nodiscard]] dijkstra_result_type
  shortest_path_tree_linear(
    std::function<type_weight(edge_type const &)> &weights,
    std::size_t                                    devices) const
  {
    return dijkstra_linear(weights, graph.nodes.size() - 1, true, devices);
  } // time: ((N+E)log(N)), space: O(N)


protected:
  Graph<T, id_content> const &graph;


  /// Given the result of the dijkstra algorithm, it will return the shortest
  /// path from the root to the final node
  /// \param dij_res The result of the dijkstra algorithm
  /// \param root The starting node
  /// \return The shortest path
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

private:
  /// Given a node_id, it will produce it's children in the current graph
  /// \param node_id The node id
  /// \param reversed If true, every edge is considered reversed
  /// \return The children of the given node
  [[nodiscard]] std::set<node_id_type> const &
  extract_children(node_id_type const &node_id, bool const &reversed) const
  {
    return reversed ? graph.dependencies[node_id].first :
                      graph.dependencies[node_id].second;
  }

  /// Given the tail and the head of the edge, it will produce the associated
  /// weight \param tail The tail node id \param head The head node id \param
  /// weight_fun The weight function \param reversed If true, every edge is
  /// considered reversed \return The corresponding weight
  type_weight
  get_weight(std::size_t                                    tail,
             std::size_t                                    head,
             std::function<type_weight(edge_type const &)> &weight_fun,
             bool const                                    &reversed) const
  {
    edge_type edge =
      reversed ? std::make_pair(head, tail) : std::make_pair(tail, head);

    return weight_fun(edge);
  }
};


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
