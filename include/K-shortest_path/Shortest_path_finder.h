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


namespace network_butcher_kfinder
{

  /// A simple (static) class that performs the dijkstra on the given graph
  /// \tparam Graph_type The type of the graph
  template <class Graph_type>
  class Shortest_path_finder
  {
  public:
    /// Executes dijkstra algorithm to compute the shortest paths from the root
    /// to evert node for the given graph
    /// \param graph The graph
    /// \param root The starting vertex
    /// \param reversed Reverses the edge directions
    /// \return A pair: the first element is the collection of the successors (along the shortest path) of the different
    /// nodes while the second element is the shortest path length from the root to every node
    [[nodiscard]] static dijkstra_result_type
    dijkstra(Graph_type const &graph,
             node_id_type      root     = 0,
             bool              reversed = false) // time: ((N+E)log(N)), space: O(N)
    {
      auto const &nodes = graph.get_nodes();

      if (nodes.empty())
        return {{}, {}};

      std::vector<weight_type> total_distance(nodes.size(), std::numeric_limits<double>::max());
      total_distance[root] = 0;

      std::vector<node_id_type>        predecessors(nodes.size(), root);
      std::set<dijkstra_helper_struct> to_visit{{0, root}};
      auto const                      &dependencies = graph.get_dependencies();

      while (!to_visit.empty()) // O(N)
        {
          auto current_node = *to_visit.begin(); // O(1)

          auto const &start_distance = total_distance[current_node.id];
          if (start_distance == std::numeric_limits<weight_type>::max())
            {
              std::cout << "Dijkstra error: the current distance is +inf" << std::endl;
              return {predecessors, total_distance};
            }

          to_visit.erase(to_visit.begin()); // O(log(N))

          auto const &children = extract_children(graph, current_node.id, reversed);
          if (!children.empty())
            {
              for (auto const &head_node : children)
                {
                  auto      &base_distance = total_distance[head_node]; // O(1)
                  auto const weight        = get_weight(graph, current_node.id, head_node, reversed);

                  if (weight < 0)
                    {
                      if (!reversed)
                        std::cout << "Error: missing weight (" << current_node.id << ", " << head_node << ")"
                                  << std::endl;
                      else
                        std::cout << "Error: missing weight (" << head_node << ", " << current_node.id << ")"
                                  << std::endl;
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
        }

      return {predecessors, total_distance};
    }

    /// Computes through dijkstra the shortest path single destination tree for
    /// the given graph
    /// \param graph The graph
    /// \return A pair: the first element is the collection of the successors
    /// (along the shortest path) of the different nodes while the second
    /// element is the shortest path length from every node to the sink
    [[nodiscard]] static dijkstra_result_type
    shortest_path_tree(Graph_type const &graph)
    {
      return dijkstra(graph, graph.size() - 1, true);
    } // time: ((N+E)log(N)), space: O(N)

    /// Given the result of the dijkstra algorithm, it will return the shortest
    /// path from the root to the final node
    /// \param graph The graph
    /// \param dij_res The result of the dijkstra algorithm
    /// \param root The starting node
    /// \return The shortest path
    static path_info
    shortest_path_finder(Graph_type const                                                     &graph,
                         std::pair<std::vector<node_id_type>, std::vector<weight_type>> const &dij_res,
                         node_id_type                                                          root)
    {
      path_info info;
      info.length = dij_res.second[root];
      info.path.reserve(graph.size());

      auto ind = root;
      while (ind != graph.get_nodes().back().get_id())
        {
          info.path.push_back(ind);
          ind = dij_res.first[ind];
        }
      info.path.push_back(ind);

      return info;
    }

  private:
    /// Given a node_id, it will produce it's children in the current graph
    /// \param graph The graph
    /// \param node_id The node id
    /// \param reversed If true, every edge is considered reversed
    /// \return The children of the given node
    [[nodiscard]] static std::set<node_id_type> const &
    extract_children(Graph_type const &graph, node_id_type const &node_id, bool const &reversed)
    {
      auto const &dependencies = graph.get_dependencies();
      return reversed ? dependencies[node_id].first : dependencies[node_id].second;
    }

    /// Given the tail and the head of the edge, it will produce the associated
    /// weight
    /// \param graph The graph
    /// \param tail The tail node id
    /// \param head The head node id
    /// \param weight_map The weight map
    /// \param reversed If true, every edge is considered reversed
    /// \return The corresponding weight
    static weight_type
    get_weight(Graph_type const &graph, std::size_t tail, std::size_t head, bool const &reversed)
    {
      edge_type edge = reversed ? std::make_pair(head, tail) : std::make_pair(tail, head);

      return graph.get_weigth(edge);
    }
  };
} // namespace network_butcher_kfinder


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
