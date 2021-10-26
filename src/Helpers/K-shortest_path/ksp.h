//
// Created by faccus on 26/10/21.
//

#ifndef NETWORK_BUTCHER_KSP_H
#define NETWORK_BUTCHER_KSP_H

#include "../Traits/Graph_traits.h"
#include <limits>


template <class T>
class KFinder
{
private:
  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  struct dijkstra_helper_struct
  {
    type_weight  weight;
    node_id_type id;

    constexpr bool
    operator<(const dijkstra_helper_struct &rhs) const
    {
      return weight < rhs.weight && (weight == rhs.weight && id < rhs.id);
    }
  };

  Graph<T> const                &graph;
  type_collection_weights const &weights;

public:
  explicit KFinder(Graph<T> const &g, type_collection_weights const &w)
    : graph(g)
    , weights(w){};

  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  dijkstra() const // time: ((N+E)log(N)), space: O(N)
  {
    if (graph.nodes.empty())
      return {};

    std::vector<type_weight> total_distance(
      graph.nodes.size(), std::numeric_limits<type_weight>::max());
    *total_distance.begin() = 0;

    std::vector<node_id_type> predecessors(graph.nodes.size(),
                                           graph.nodes.front());

    std::set<dijkstra_helper_struct> to_visit{graph.nodes.front()};

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

        auto &exit_nodes = graph.dependencies[current_node.id].second; // O(1)
        for (auto &j : exit_nodes)
          {
            auto      &basic_dist = total_distance[j]; // O(1)
            auto const candidate_distance =
              start_distance + weights[{current_node.id, j}]; // O(1)
            if (candidate_distance < basic_dist)              // O(1)
              {
                to_visit.erase({j, basic_dist}); // O(log(N))

                predecessors[j] = current_node.id;        // O(1)
                basic_dist      = candidate_distance;     // O(1)
                to_visit.insert({j, candidate_distance}); // O(log(N))
              }
          }
      }

    return {predecessors, total_distance};
  }
};


#endif // NETWORK_BUTCHER_KSP_H
