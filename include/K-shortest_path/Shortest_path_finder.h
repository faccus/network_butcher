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
  template <typename Weight_Type = weight_type>
  struct dijkstra_helper_struct : crtp_greater<dijkstra_helper_struct<Weight_Type>>
  {
    Weight_Type  weight;
    node_id_type id;

    dijkstra_helper_struct(Weight_Type w, node_id_type i)
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
  template <typename Weight_Type = weight_type>
  using dijkstra_result_type = std::pair<std::vector<node_id_type>, std::vector<Weight_Type>>;

  namespace utilities
  {
    /// Given the tail and the head of the edge, it will produce the associated weight
    /// \param graph The graph
    /// \param tail The tail node id
    /// \param head The head node id
    /// \return The corresponding weight
    template <Valid_Weighted_Graph v_Weighted_Graph>
    v_Weighted_Graph::Weight_Type
    get_weight(v_Weighted_Graph const &graph, node_id_type tail, node_id_type head)
    {
      auto const &weight_container = graph.get_weight(std::make_pair(tail, head));

      return *weight_container.cbegin();
    }
  } // namespace utilities


  /// Executes dijkstra algorithm to compute the shortest paths from the root to every node of the input graph
  /// \param graph The graph
  /// \param root The starting vertex
  /// \return The struct dijkstra_result_type<v_Weighted_Graph::Weight_Type>
  template <Valid_Weighted_Graph v_Weighted_Graph>
  [[nodiscard]] dijkstra_result_type<typename v_Weighted_Graph::Weight_Type>
  dijkstra(v_Weighted_Graph const &graph,
           node_id_type            root = 0) // time: ((N+E)log(N)), space: O(N)
  {
    using dijkstra_result_type   = dijkstra_result_type<typename v_Weighted_Graph::Weight_Type>;
    using dijkstra_helper_struct = dijkstra_helper_struct<typename v_Weighted_Graph::Weight_Type>;

    if (graph.empty())
      {
        return dijkstra_result_type{};
      }

    std::vector<typename v_Weighted_Graph::Weight_Type> total_distance(
      graph.size(), std::numeric_limits<typename v_Weighted_Graph::Weight_Type>::max());
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
        if (start_distance == std::numeric_limits<typename v_Weighted_Graph::Weight_Type>::max())
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
  template <Valid_Weighted_Graph v_Weighted_Graph>
  t_path_info<typename v_Weighted_Graph::Weight_Type>
  shortest_path_finder(v_Weighted_Graph const                                             &graph,
                       dijkstra_result_type<typename v_Weighted_Graph::Weight_Type> const &dij_res,
                       node_id_type                                                        root,
                       node_id_type                                                        sink)
  {
    t_path_info info;
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
