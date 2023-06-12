#ifndef NETWORK_BUTCHER_SHORTEST_PATH_FINDER_H
#define NETWORK_BUTCHER_SHORTEST_PATH_FINDER_H

#include <network_butcher/K-shortest_path/heap_traits.h>

#include <limits>
#include <queue>
#include <vector>

namespace network_butcher::kfinder::Shortest_path_finder::utilities
{
  /// A helper struct for the Dijkstra algorithm (we lose aggregate status, but we can use the emplace methods of STL
  /// containers)
  template <typename Weight_Type = Time_Type>
  struct Dijkstra_Helper : Crtp_Greater<Dijkstra_Helper<Weight_Type>>
  {
    /// Weight of the node
    Weight_Type weight;

    /// Node id
    Node_Id_Type id;

    /// Simple constructor
    /// \param w Weight
    /// \param i Index
    Dijkstra_Helper(Weight_Type w, Node_Id_Type i)
      : weight(w)
      , id(i)
    {}

    bool
    operator<(const Dijkstra_Helper &rhs) const
    {
      return weight < rhs.weight || (weight == rhs.weight && id < rhs.id);
    }
  };

  /// Given a pair of nodes, it will produce the smallest weight associated to one of the edges made by the pair
  /// \param graph The graph
  /// \param tail The tail node id
  /// \param head The head node id
  /// \return The corresponding weight
  template <Valid_Weighted_Graph v_Weighted_Graph>
  auto
  get_weight(v_Weighted_Graph const &graph, Node_Id_Type tail, Node_Id_Type head) -> v_Weighted_Graph::Weight_Type
  {
    auto const &weight_container = graph.get_weight(std::make_pair(tail, head));

    return *weight_container.cbegin();
  }
} // namespace network_butcher::kfinder::Shortest_path_finder::utilities

namespace network_butcher::kfinder::Shortest_path_finder
{
  /// The output type of the Dijkstra algorithm
  /// \tparam Weight_Type The weight type
  template <typename Weight_Type = Time_Type>
  using Templated_Dijkstra_Result_Type = std::pair<std::vector<Node_Id_Type>, std::vector<Weight_Type>>;


  /// Executes Dijkstra algorithm to compute the shortest paths from the root to every node of the graph. The overall
  /// time complexity should be O((N+E)*log(N))
  /// \tparam v_Weighted_Graph The weighted graph type
  /// \param graph The graph
  /// \param root The starting vertex
  /// \return The collection of successor and shortest distances
  template <Valid_Weighted_Graph v_Weighted_Graph>
  [[nodiscard]] auto
  dijkstra(v_Weighted_Graph const &graph,
           Node_Id_Type            root)
    -> Templated_Dijkstra_Result_Type<typename v_Weighted_Graph::Weight_Type> // time: ((N+E)log(N))
  {
    using Weight_Type            = typename v_Weighted_Graph::Weight_Type;
    using dijkstra_result_type   = Templated_Dijkstra_Result_Type<Weight_Type>;
    using dijkstra_helper_struct = utilities::Dijkstra_Helper<Weight_Type>;

    if (graph.empty())
      {
        return dijkstra_result_type{};
      }

    std::vector<Weight_Type> total_distance(graph.size(), std::numeric_limits<Weight_Type>::max()); // O(N)
    total_distance[root] = 0;

    std::vector<Node_Id_Type> predecessors(graph.size(), std::numeric_limits<Node_Id_Type>::max()); // O(N)
    predecessors[root] = root;

    // We are using a set instead of a priority_queue to have O(log(N)) erase
    std::set<dijkstra_helper_struct> to_visit;
    to_visit.emplace(0, root);

    auto const error_message = [](auto const &tail, auto const &head) {
      std::stringstream error_msg;
      error_msg << "Error: either the weight (";

      error_msg << tail << ", " << head;

      error_msg << ")"
                << "is negative" << std::endl;

      return error_msg.str();
    };

    // The complexity due to the loop is considered below
    while (!to_visit.empty())
      {
        auto current_node =
          std::move(to_visit.extract(to_visit.begin()).value());      // O(N*log(N)), O(log(N)) up to once per node

        auto const &start_distance = total_distance[current_node.id]; // O(N), O(1) once per node
        if (start_distance == std::numeric_limits<Weight_Type>::max())
          {
            throw std::logic_error("Dijkstra error: the node current distance is +inf");
          }

        for (auto const &head_node :
             graph.get_output_nodes(current_node.id)) // O(M) taking into account the previous loop
          {
            if (head_node == current_node.id)
              continue;

            auto      &base_distance = total_distance[head_node];       // O(1)
            auto const weight =
              utilities::get_weight(graph, current_node.id, head_node); // Up to O(log(N)) base on the underlying graph

            if (weight < 0)
              {
                throw std::logic_error(error_message(current_node.id, head_node));
              }

            auto const candidate_distance = start_distance + weight;              // O(1)
            if (candidate_distance < base_distance)                               // O(1)
              {
                to_visit.erase(dijkstra_helper_struct(base_distance, head_node)); // O(log(N)

                predecessors[head_node] = current_node.id;                        // O(1)
                base_distance           = candidate_distance;                     // O(1)
                to_visit.emplace(candidate_distance, head_node);                  // O(log(N))
              }
          }
      }

    return {predecessors, total_distance};
  }


  /// Given the result of the Dijkstra algorithm, it will return the shortest path from the root to the final node
  /// \param graph The graph
  /// \param dij_res The result of the dijkstra algorithm
  /// \param root The starting node
  /// \param sink The ending node
  /// \return The shortest path
  template <Valid_Weighted_Graph v_Weighted_Graph>
  auto
  shortest_path_finder(v_Weighted_Graph const                                                       &graph,
                       Templated_Dijkstra_Result_Type<typename v_Weighted_Graph::Weight_Type> const &dij_res,
                       Node_Id_Type                                                                  root,
                       Node_Id_Type sink) -> Templated_Path_Info<typename v_Weighted_Graph::Weight_Type>
  {
    Templated_Path_Info<typename v_Weighted_Graph::Weight_Type> info;
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
