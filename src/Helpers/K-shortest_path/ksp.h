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
  struct implicit_path_info
  {
    std::vector<edge_type> sidetracks;
    type_weight            length;

    constexpr bool
    operator<(const implicit_path_info &rhs) const
    {
      return length < rhs.length ||
             (length == rhs.length && sidetracks < rhs.sidetracks);
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

  [[nodiscard]] std::set<implicit_path_info>
  basic_eppstein(int K = 2)
  {
    auto const dij_res = shortest_path_tree();

    std::set<implicit_path_info> res;
    res.insert(res.cend(), {{}, dij_res.second.front()});

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return res;


    auto const sidetrack_distances_res = sidetrack_distances(dij_res.second);
    auto const shortest_path           = shortest_path_finder(dij_res);

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    std::map<node_id_type, H_g_content> h_out =
      construct_h_out(successors, sidetrack_distances_res);

    std::pair<std::map<node_id_type, H_g>,
              std::map<edge_type, std::set<edge_type>>>
      pair = construct_h_g(h_out, successors);

    auto &h_g         = pair.first;
    auto &edges_edges = pair.second;

    auto const first_side_track = side_track(0, h_g);

    std::priority_queue<implicit_path_info> Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.push(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = Q.top();
        Q.pop();
        res.insert(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
          }

        {
          auto       mod_sk = SK;
          auto const f      = side_track(e.second, h_g);

          mod_sk.sidetracks.push_back(f.edge);
          mod_sk.length += f.delta_weight;
          Q.push(std::move(mod_sk));
        }
        auto const it = edges_edges.find(e);
        if (it != edges_edges.cend())
          {
            SK.sidetracks.pop_back();

            for (auto &f : it->second)
              {
                auto ut = sidetrack_distances_res.find(f);

                if (ut == sidetrack_distances_res.cend())
                  {
                    std::cout << "Error: cannot find proper sidetrack distance "
                                 "for edge ("
                              << f.first << ", " << f.second << ")"
                              << std::endl;
                  }

                auto mod_sk = SK;
                mod_sk.sidetracks.push_back(f);
                mod_sk.length += (ut->second - ot->second);

                Q.push(std::move(mod_sk));
              }
          }
      }


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


  struct H_helper
  {
    edge_type   edge;
    type_weight delta_weight;
    constexpr bool
    operator<(const H_helper &rhs) const
    {
      return delta_weight < rhs.delta_weight ||
             (delta_weight == rhs.delta_weight && edge < rhs.edge);
    }
  };
  struct H_out
  {
    Heap<H_helper> heap;

    constexpr bool
    operator<(const H_out &rhs) const
    {
      if (!heap.children.empty() && !rhs.heap.children.empty())
        return *heap.children.cbegin() < *rhs.heap.children.cbegin();
      else if (heap.children.empty() && rhs.heap.children.empty())
        return true;
      else
        return heap.children.empty();
    }
  };
  struct H_g_content
  {
    std::shared_ptr<H_out> content;

    constexpr bool
    operator<(const H_g_content &rhs) const
    {
      return *content < *rhs.content;
    }
  };
  struct H_g
  {
    Heap<H_g_content> heap;

    constexpr bool
    operator<(const H_g &rhs) const
    {
      if (heap.children.empty() && rhs.children.empty())
        return true;
      else if (!heap.children.empty() && !rhs.children.empty())
        return *heap.children.cbegin() < *rhs.children.cbegin();
      else
        return heap.children.empty();
    }
  };


  Graph<T> const                &graph;
  type_collection_weights const &weights;

  H_helper
  side_track(node_id_type const &j, std::map<node_id_type, H_g> const &h_g)
  {
    auto const it = h_g.find(j);
    if (it == h_g.cend() || it->second.heap.children.empty())
      return {};

    return *it->second.heap.children.begin()->content->heap.children.begin();
  }

  std::map<node_id_type, H_g_content>
  construct_h_out(std::vector<node_id_type> const &successors,
                  type_collection_weights const   &sidetrack_distances) const
  {
    std::map<node_id_type, H_g_content> h_out;

    // H_out
    for (auto &node : graph.nodes)
      {
        auto const &exit_nodes = graph.dependencies[node.get_id()].second;

        H_g_content h_out_tmp;
        h_out_tmp.content = std::make_shared<H_out>();
        auto &heap        = h_out_tmp.content;

        if (!exit_nodes.empty())
          {
            auto &succ = successors[node.get_id()];
            for (auto const &exit_node : exit_nodes) // O(E)
              {
                if (exit_node != succ)
                  {
                    H_helper tmp;
                    tmp.edge      = {node.get_id(), exit_node};
                    auto const it = sidetrack_distances.find(tmp.edge);

                    if (it != sidetrack_distances.cend())
                      {
                        tmp.delta_weight = it->second;

                        heap->heap.children.insert(std::move(tmp)); // O(log(E))
                      }
                  }
              }
          }

        h_out.insert({node.get_id(), std::move(h_out_tmp)});
      }

    return h_out;
  }

  std::pair<std::map<node_id_type, H_g>,
            std::map<edge_type, std::set<edge_type>>>
  construct_h_g(std::map<node_id_type, H_g_content> &h_out,
                std::vector<node_id_type> const     &successors)
  {
    std::map<node_id_type, H_g> res;

    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(graph.nodes.size());

    for (auto &node : graph.nodes)
      {
        auto &tmp = successors[node.get_id()];

        if (tmp != node.get_id())
          sp_dependencies[tmp].insert(node.get_id());
      }

    H_g heap_last_node;

    if (!h_out[graph.nodes.size() - 1].content->heap.children.empty())
      heap_last_node.heap.children.insert({h_out[graph.nodes.size() - 1]});
    res.insert({graph.nodes.size() - 1, std::move(heap_last_node)});

    std::queue<node_id_type> queue;

    for (auto it = ++graph.nodes.crbegin(); it != graph.nodes.crend(); ++it)
      if (successors[it->get_id()] == graph.nodes.size() - 1)
        queue.push(it->get_id());

    while (!queue.empty())
      {
        auto &deps = sp_dependencies[queue.front()];

        for (auto &n : deps)
          queue.push(n);

        H_g heap_node;

        if (!h_out[queue.front()].content->heap.children.empty())
          heap_node.heap.children.insert({h_out[queue.front()]});

        {
          auto const &tmo = res[successors[queue.front()]].heap.children;

          if (!tmo.empty())
            heap_node.heap.children.insert(tmo.begin(), tmo.end());
        }

        res.insert({queue.front(), std::move(heap_node)});

        queue.pop();
      }

    std::map<edge_type, std::set<edge_type>> edge_edges;
    for (auto it = res.begin(); it != res.cend(); ++it)
      {
        auto const &h_g_tm = *it;
        auto const &h_g    = h_g_tm.second;
        std::size_t j      = 0;
        std::vector<typename std::set<H_g_content>::const_iterator>
          previous_external;

        for (auto it2 = h_g.heap.children.begin();
             it2 != h_g.heap.children.cend();
             ++it2)
          {
            auto const &internal_content = *it2;
            previous_external.push_back(it2);

            if (j > 0)
              {
                std::size_t external_parent = (j - 1) / 2;
                if (external_parent != j)
                  {
                    auto &edge_index = (*previous_external[external_parent]
                                           ->content->heap.children.begin())
                                         .edge;
                    edge_edges[edge_index].insert(
                      (*internal_content.content->heap.children.begin()).edge);
                  }
              }

            std::size_t i = 0;
            std::vector<typename std::set<H_helper>::const_iterator>
              previous_internal;

            for (auto it3 = internal_content.content->heap.children.begin();
                 it3 != internal_content.content->heap.children.end();
                 ++it3)
              {
                auto const &sidetrack_edge = *it3;
                previous_internal.push_back(it3);

                if (i > 0)
                  {
                    std::size_t internal_parent = (i - 1) / 2;
                    if (internal_parent != i)
                      {
                        auto &edge_index =
                          previous_internal[internal_parent]->edge;
                        edge_edges[edge_index].insert(sidetrack_edge.edge);
                      }
                  }

                ++i;
              }

            ++j;
          }
      }

    return {res, edge_edges};
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
  shortest_path_finder(std::pair<std::vector<node_id_type>,
                                 std::vector<type_weight>> const &dij_res,
                       node_id_type root = 0) const
  {
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
