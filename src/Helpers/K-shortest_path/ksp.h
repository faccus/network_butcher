//
// Created by faccus on 26/10/21.
//

#ifndef NETWORK_BUTCHER_KSP_H
#define NETWORK_BUTCHER_KSP_H

#include "../Traits/Graph_traits.h"
#include "Heap.h"

#include <limits>
#include <queue>
#include <vector>


template <class T>
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

  explicit KFinder(Graph<T> const &g, type_collection_weights const &w)
    : graph(g)
    , weights(w){};


  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  dijkstra(node_id_type root = 0,
           bool reversed     = false) const // time: ((N+E)log(N)), space: O(N)
  {
    if (graph.nodes.empty())
      return {{}, {}};

    std::vector<type_weight> total_distance(graph.nodes.size(),
                                            std::numeric_limits<double>::max());
    total_distance[root] = 0;

    std::vector<node_id_type>        predecessors(graph.nodes.size(), root);
    std::set<dijkstra_helper_struct> to_visit{{0, root}};

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

        auto &exit_nodes = reversed ?
                             graph.dependencies[current_node.id].first :
                             graph.dependencies[current_node.id].second; // O(1)

        for (auto j : exit_nodes)
          {
            auto &basic_dist = total_distance[j]; // O(1)
            auto  ref        = reversed ? weights.find({j, current_node.id}) :
                                          weights.find({current_node.id, j});

            if (ref == weights.cend())
              {
                std::cout << "Error: missing weight (" << current_node.id
                          << ", " << j << ")" << std::endl;
                return {predecessors, total_distance};
              }

            auto const candidate_distance =
              start_distance + ref->second;      // O(1)
            if (candidate_distance < basic_dist) // O(1)
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
  shortest_path_tree() const
  {
    return dijkstra(graph.nodes.size() - 1, true);
  }

  [[nodiscard]] std::set<path_info>
  lazy_lazy_eppstein(int k = 1)
  {
    if (graph.nodes.empty())
      return {};
    if (k == 1)
      return {shortest_path_finder()};

    std::set<path_info> res;

    auto const dij_res                 = shortest_path_tree();
    auto const sidetrack_distances_res = sidetrack_distances(dij_res.second);
    auto const shortest_path           = shortest_path_finder(dij_res);

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    std::set<path_info> paths{shortest_path};

    std::map<node_id_type, std::shared_ptr<Heap<H_out_helper>>> h_out =
      construct_h_out(successors, sidetrack_distances_res);
    std::map<node_id_type, std::shared_ptr<H_out>> h_g =
      construct_h_g(h_out, successors);


    return res;
  }

private:
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
  struct H_out_helper
  {
    edge_type   edge;
    type_weight delta_weight;
    constexpr bool
    operator<(const H_out_helper &rhs) const
    {
      return delta_weight < rhs.delta_weight ||
             (delta_weight == rhs.delta_weight && edge < edge);
    }
  };

  using H_out = std::shared_ptr<H_out_helper>;


  Graph<T> const                &graph;
  type_collection_weights const &weights;


  std::map<node_id_type, std::shared_ptr<Heap<H_out_helper>>>
  construct_h_out(std::vector<node_id_type> const &successors,
                  type_collection_weights const   &sidetrack_distances) const
  {
    std::map<node_id_type, std::shared_ptr<Heap<H_out_helper>>> h_out;

    // H_out
    for (auto &node : graph.nodes)
      {
        auto const &exit_nodes = graph.dependencies[node.get_id()].second;
        std::shared_ptr<Heap<H_out_helper>> heap;

        if (!exit_nodes.empty())
          {
            auto &succ = successors[node.get_id()];
            for (auto const &exit_node : exit_nodes) // O(E)
              {
                if (exit_node != succ)
                  {
                    H_out_helper tmp;
                    tmp.edge         = {node.get_id(), exit_node};
                    tmp.delta_weight = sidetrack_distances[tmp.edge];

                    heap->children.insert(tmp); // O(log(E))
                  }
              }
          }

        h_out.insert(node.get_id(), heap);
      }

    return h_out;
  }

  std::map<node_id_type, std::shared_ptr<Heap<std::shared_ptr<Heap<H_out>>>>>
  construct_h_g(
    std::map<node_id_type, std::shared_ptr<Heap<H_out_helper>>> &h_out,
    std::vector<node_id_type> const                             &successors)
  {
    std::map<node_id_type, std::shared_ptr<Heap<std::shared_ptr<Heap<H_out>>>>>
      res;

    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(graph.nodes.size());

    for (auto &node : graph.nodes)
      {
        auto &tmp = successors[node.get_id()];

        if (tmp != node.get_id())
          sp_dependencies[tmp].insert(node.get_id());
      }

    res.insert(graph.nodes.size() - 1,
               std::make_shared<Heap<std::shared_ptr<Heap<H_out>>>>());
    res[graph.nodes.size() - 1]->merge(h_out[graph.nodes.size() - 1]);
    std::queue<node_id_type> queue;

    for (auto &node : graph.nodes)
      if (successors[node.get_id] == graph.nodes.size() - 1)
        queue.push(node.get_id());

    while (!queue.empty())
      {
        auto &deps = sp_dependencies[queue.front()];

        for (auto &n : deps)
          queue.push(n);

        res.insert(queue.front(),
                   std::make_shared<Heap<std::shared_ptr<Heap<H_out>>>>());
        res[queue.front()]->merge(h_out[queue.front()]);
        res[queue.front()]->merge(*res[successors[queue.front()]]);

        queue.pop();
      }

    return res;
  }


  [[nodiscard]] type_collection_weights
  sidetrack_distances(std::vector<type_weight> const &distances_from_sink) const
  {
    type_collection_weights res;
    for (auto const &edge : weights) // O(E)
      {
        res.insert(res.cend(),
                   {edge.first,
                    edge.second + distances_from_sink[edge.first.second] -
                      distances_from_sink[edge.first.first]}); // O(1)
      }

    return res;
  }

  path_info
  shortest_path_finder(
    node_id_type root = 0,
    std::pair<std::vector<node_id_type>, std::vector<type_weight>> const
      &infos_succ =
        std::pair<std::vector<node_id_type>, std::vector<type_weight>>()) const
  {
    auto const &dij_res =
      !infos_succ.first.empty() && !infos_succ.second.empty() ?
        infos_succ :
        dijkstra(graph.nodes.size() - 1, true);

    path_info info;
    info.length = dij_res.second[root];

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


#endif // NETWORK_BUTCHER_KSP_H
