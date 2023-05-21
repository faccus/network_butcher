//
// Created by faccus on 26/10/21.
//

#ifndef NETWORK_BUTCHER_SHORTEST_PATH_FINDER_H
#define NETWORK_BUTCHER_SHORTEST_PATH_FINDER_H

#include "Heap_traits.h"

#include <limits>
#include <queue>
#include <vector>


namespace network_butcher::kfinder::Shortest_path_finder
{
  /// A helper struct for the dijkstra algo
  struct dijkstra_helper_struct : crtp_greater<dijkstra_helper_struct>
  {
    weight_type  weight;
    node_id_type id;

    dijkstra_helper_struct(weight_type w, node_id_type i)
      : weight(w)
      , id(i)
    {}

    constexpr bool
    operator<(const dijkstra_helper_struct &rhs) const
    {
      return weight < rhs.weight || (weight == rhs.weight && id < rhs.id);
    }
  };

  /// The output type of the Dijkstra algorithm
  using dijkstra_result_type = std::pair<std::vector<node_id_type>, std::vector<weight_type>>;

  namespace utilities
  {
    /// Given the tail and the head of the edge, it will produce the associated weight
    /// \param graph The graph
    /// \param tail The tail node id
    /// \param head The head node id
    /// \return The corresponding weight
    template <typename Graph_type,
              bool Reversed                 = false,
              typename Node_Type            = typename Graph_type::Node_Type,
              typename Node_Collection_Type = typename Graph_type::Node_Collection_Type,
              typename Dependencies_Type    = typename Graph_type::Dependencies_Type>
    weight_type
    get_weight(
      Weighted_Graph<Graph_type, Node_Type, Node_Collection_Type, Dependencies_Type, Reversed> const &graph,
      typename Weighted_Graph<Graph_type, Node_Type, Node_Collection_Type, Dependencies_Type, Reversed>::Node_Id_Type
        tail,
      typename Weighted_Graph<Graph_type, Node_Type, Node_Collection_Type, Dependencies_Type, Reversed>::Node_Id_Type
        head)
    {
      auto const &weight_container = graph.get_weight(std::make_pair(tail, head));

      return *weight_container.cbegin();
    }
  } // namespace utilities


  /// Executes dijkstra algorithm to compute the shortest paths from the root to every node of the input graph
  /// \param graph The graph
  /// \param root The starting vertex
  /// \param reversed Reverses the edge directions
  /// \return A pair: the first element is the collection of the successors (along the shortest path) of the different
  /// nodes while the second element is the shortest path length from the root to every node
  template <typename Graph_type,
            bool Reversed                 = false,
            typename Node_Type            = typename Graph_type::Node_Type,
            typename Node_Collection_Type = typename Graph_type::Node_Collection_Type,
            typename Dependencies_Type    = typename Graph_type::Dependencies_Type>
  [[nodiscard]] dijkstra_result_type
  dijkstra(
    Weighted_Graph<Graph_type, Node_Type, Node_Collection_Type, Dependencies_Type, Reversed> const &graph,
    typename Weighted_Graph<Graph_type, Node_Type, Node_Collection_Type, Dependencies_Type, Reversed>::Node_Id_Type
      root = 0) // time: ((N+E)log(N)), space: O(N)
  {
    using node_id_type = typename Weighted_Graph<Graph_type>::Node_Id_Type;

    if (graph.empty())
      {
        return dijkstra_result_type{};
      }

    std::vector<weight_type> total_distance(graph.size(), std::numeric_limits<network_butcher::weight_type>::max());
    total_distance[root] = 0;


    std::vector<node_id_type> predecessors(graph.size(), std::numeric_limits<node_id_type>::max());
    predecessors[root] = root;

    Heap<dijkstra_helper_struct, std::greater<>> to_visit;

    to_visit.emplace(0, root);

    auto const error_message = [](auto const &tail, auto const &head) {
      std::stringstream error_msg;
      error_msg << "Error: either the weight (";

      error_msg << tail << ", " << head;

      error_msg << ")"
                << "is missing or it's negative" << std::endl;

      return error_msg.str();
    };

    while (!to_visit.empty())                    // O(N)
      {
        auto current_node = to_visit.pop_head(); // O(log(N))

        auto const &start_distance = total_distance[current_node.id];
        if (start_distance == std::numeric_limits<weight_type>::max())
          {
            throw std::logic_error("Dijkstra error: the node current distance is +inf");
          }

        if (current_node.weight != total_distance[current_node.id])
          continue;

        for (auto const &head_node : graph.get_output_nodes(current_node.id))
          {
            if (head_node == current_node.id)
              continue;

            auto &base_distance = total_distance[head_node]; // O(1)

            auto const &w = graph.get_weight(std::make_pair(current_node.id, head_node));

            auto const weight = utilities::get_weight(graph, current_node.id, head_node);

            if (weight < 0)
              {
                throw std::logic_error(error_message(current_node.id, head_node));
              }

            auto const candidate_distance = start_distance + weight; // O(1)
            if (candidate_distance < base_distance)                  // O(1)
              {
                predecessors[head_node] = current_node.id;           // O(1)
                base_distance           = candidate_distance;        // O(1)
                to_visit.emplace(candidate_distance, head_node);     // O(log(N))
              }
          }
      }

    return {predecessors, total_distance};
  }


  /// Given the result of the dijkstra algorithm, it will return the shortest path from the root to the final node
  /// \param graph The graph
  /// \param dij_res The result of the dijkstra algorithm
  /// \param root The starting node
  /// \param sink The ending node
  /// \return The shortest path
  template <typename Graph_type,
            bool Reversed                 = false,
            typename Node_Type            = typename Graph_type::Node_Type,
            typename Node_Collection_Type = typename Graph_type::Node_Collection_Type,
            typename Dependencies_Type    = typename Graph_type::Dependencies_Type>
  path_info
  shortest_path_finder(
    Weighted_Graph<Graph_type, Node_Type, Node_Collection_Type, Dependencies_Type, Reversed> const &graph,
    dijkstra_result_type const                                                                     &dij_res,
    typename Weighted_Graph<Graph_type, Node_Type, Node_Collection_Type, Dependencies_Type, Reversed>::Node_Id_Type
      root,
    typename Weighted_Graph<Graph_type, Node_Type, Node_Collection_Type, Dependencies_Type, Reversed>::Node_Id_Type
      sink)
  {
    path_info info;
    info.length = dij_res.second[root];
    info.path.reserve(graph.size());

    auto ind = root;
    while (ind != sink)
      {
        info.path.push_back(ind);
        ind = dij_res.first[ind];
      }
    info.path.push_back(ind);

    return info;
  }
}; // namespace network_butcher::kfinder::Shortest_path_finder


#endif // NETWORK_BUTCHER_SHORTEST_PATH_FINDER_H
