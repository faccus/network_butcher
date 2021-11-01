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


template <class T>
class KFinder
{
public:
  using H_out_map = std::map<node_id_type, H_out_pointer>;

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

    constexpr bool
    operator>(const implicit_path_info &rhs) const
    {
      return length > rhs.length ||
             (length == rhs.length && sidetracks > rhs.sidetracks);
    }
  };

  explicit KFinder(Graph<T> const &g)
    : graph(g){};


  [[nodiscard]] std::pair<std::vector<node_id_type>, std::vector<type_weight>>
  dijkstra(type_collection_weights const &weights,
           node_id_type                   root = 0,
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
                std::cout << "Error: missing weight (" << current_node.id
                          << ", " << j << ")" << std::endl;
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
    if (devices == 1)
      return dijkstra(weights, root, reversed);

    if (graph.nodes.empty() || devices == 0)
      return {{}, {}};


    std::vector<type_weight> total_distance(graph.nodes.size() * devices,
                                            std::numeric_limits<double>::max());
    total_distance[root] = 0;

    std::vector<node_id_type> predecessors(graph.nodes.size() * devices, root);
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

        auto exit_nodes = reversed ?
                            dependencies[current_node.id % num_nodes].first :
                            dependencies[current_node.id % num_nodes].second;

        for (auto j : exit_nodes) // O(E)
          {
            for (std::size_t k = 0; k < devices; ++k)
              {
                auto tail = current_node.id;
                auto head = j + k * num_nodes;

                auto &basic_dist = total_distance[head]; // O(1)
                auto  ref        = reversed ? weights.find({head, tail}) :
                                              weights.find({tail, head});

                if (ref == weights.cend())
                  {
                    std::cout << "Error: missing weight (" << head << ", "
                              << tail << ")" << std::endl;
                    return {predecessors, total_distance};
                  }

                auto const candidate_distance =
                  start_distance + ref->second;      // O(1)
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
          }
      }

    return {predecessors, total_distance};
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


    std::vector<type_weight> total_distance(graph.nodes.size() * devices,
                                            std::numeric_limits<double>::max());
    total_distance[root] = 0;

    std::vector<node_id_type> predecessors(graph.nodes.size() * devices, root);
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

        auto exit_nodes = reversed ?
                            dependencies[current_node.id % num_nodes].first :
                            dependencies[current_node.id % num_nodes].second;

        for (auto j : exit_nodes) // O(E)
          {
            for (std::size_t k = 0; k < devices; ++k)
              {
                auto tail = current_node.id;
                auto head = j + k * num_nodes;

                auto      &basic_dist = total_distance[head]; // O(1)
                auto const ref =
                  reversed ? weights({head, tail}) : weights({tail, head});

                if (ref < 0)
                  {
                    std::cout << "Error: missing weight (" << head << ", "
                              << tail << ")" << std::endl;
                    return {predecessors, total_distance};
                  }

                auto const candidate_distance =
                  start_distance + ref->second;      // O(1)
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
  shortest_path_tree(type_collection_weights const &weights,
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

  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein(type_collection_weights const &weights, std::size_t K)
  {
    auto const dij_res =
      shortest_path_tree(weights); // time: ((N+E)log(N)), space: O(N)

    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(K)

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return res;


    auto const sidetrack_distances_res =
      sidetrack_distances(weights, dij_res.second);              // O(E)
    auto const shortest_path = shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out = construct_h_out(successors, sidetrack_distances_res);

    auto const h_g         = construct_h_g(h_out, successors);
    auto       edges_edges = std::map<edge_type, std::set<edge_type>>();
    get_edges_edges(edges_edges, h_g);

    auto const first_side_track = side_track(0, h_g);

    std::priority_queue<implicit_path_info,
                        std::vector<implicit_path_info>,
                        std::greater<implicit_path_info>>
      Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.push(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = Q.top();
        Q.pop();
        res.push_back(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
          }

        {
          auto const f = side_track(e.second, h_g);

          if (f.edge.first >= 0 && f.edge.second >= 0)
            {
              auto mod_sk = SK;
              mod_sk.sidetracks.push_back(f.edge);
              mod_sk.length += f.delta_weight;
              Q.push(std::move(mod_sk));
            }
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


  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein_linear(type_collection_weights const &weights,
                        std::size_t                    K,
                        std::size_t                    devices)
  {
    auto const dij_res =
      shortest_path_tree_linear(weights,
                                devices); // time: ((N+E)log(N)), space: O(N)

    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(K)

    if (graph.nodes.empty() || devices == 0)
      return {};
    else if (K == 1)
      return res;
    else if (devices == 1)
      return basic_eppstein(weights, K);


    auto const sidetrack_distances_res =
      sidetrack_distances_linear(weights, devices, dij_res.second); // O(E)
    auto const shortest_path = shortest_path_finder(dij_res, 0);    // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out =
      construct_h_out_linear(successors, sidetrack_distances_res, devices);

    auto const h_g         = construct_h_g_linear(h_out, successors, devices);
    auto       edges_edges = std::map<edge_type, std::set<edge_type>>();
    get_edges_edges(edges_edges, h_g);

    auto const first_side_track = side_track(0, h_g);

    std::priority_queue<implicit_path_info,
                        std::vector<implicit_path_info>,
                        std::greater<implicit_path_info>>
      Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.push(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = Q.top();
        Q.pop();
        res.push_back(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
          }

        {
          auto const f = side_track(e.second, h_g);

          if (f.edge.first >= 0 && f.edge.second >= 0)
            {
              auto mod_sk = SK;
              mod_sk.sidetracks.push_back(f.edge);
              mod_sk.length += f.delta_weight;
              Q.push(std::move(mod_sk));
            }
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

  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein(std::function<type_weight(edge_type const &)> &weights,
                 std::size_t                                    K)
  {
    auto const dij_res =
      shortest_path_tree(weights); // time: ((N+E)log(N)), space: O(N)

    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(K)

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return res;


    auto const sidetrack_distances_res =
      sidetrack_distances(weights, dij_res.second);              // O(E)
    auto const shortest_path = shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out = construct_h_out(successors, sidetrack_distances_res);

    auto const h_g         = construct_h_g(h_out, successors);
    auto       edges_edges = std::map<edge_type, std::set<edge_type>>();
    get_edges_edges(edges_edges, h_g);

    auto const first_side_track = side_track(0, h_g);

    std::priority_queue<implicit_path_info,
                        std::vector<implicit_path_info>,
                        std::greater<implicit_path_info>>
      Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.push(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = Q.top();
        Q.pop();
        res.push_back(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
          }

        {
          auto const f = side_track(e.second, h_g);

          if (f.edge.first >= 0 && f.edge.second >= 0)
            {
              auto mod_sk = SK;
              mod_sk.sidetracks.push_back(f.edge);
              mod_sk.length += f.delta_weight;
              Q.push(std::move(mod_sk));
            }
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


  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein_linear(std::function<type_weight(edge_type const &)> &weights,
                        std::size_t                                    K,
                        std::size_t                                    devices)
  {
    auto const dij_res =
      shortest_path_tree_linear(weights,
                                devices); // time: ((N+E)log(N)), space: O(N)

    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(K)

    if (graph.nodes.empty() || devices == 0)
      return {};
    else if (K == 1)
      return res;
    else if (devices == 1)
      return basic_eppstein(weights, K);


    auto const sidetrack_distances_res =
      sidetrack_distances_linear(weights, devices, dij_res.second); // O(E)
    auto const shortest_path = shortest_path_finder(dij_res, 0);    // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out =
      construct_h_out_linear(successors, sidetrack_distances_res, devices);

    auto const h_g         = construct_h_g_linear(h_out, successors, devices);
    auto       edges_edges = std::map<edge_type, std::set<edge_type>>();
    get_edges_edges(edges_edges, h_g);

    auto const first_side_track = side_track(0, h_g);

    std::priority_queue<implicit_path_info,
                        std::vector<implicit_path_info>,
                        std::greater<implicit_path_info>>
      Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.push(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = Q.top();
        Q.pop();
        res.push_back(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
          }

        {
          auto const f = side_track(e.second, h_g);

          if (f.edge.first >= 0 && f.edge.second >= 0)
            {
              auto mod_sk = SK;
              mod_sk.sidetracks.push_back(f.edge);
              mod_sk.length += f.delta_weight;
              Q.push(std::move(mod_sk));
            }
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

  Graph<T> const &graph;

  edge_info
  side_track(node_id_type const                        &j,
             std::map<node_id_type, H_g_pointer> const &h_g)
  {
    auto const it = h_g.find(j);
    if (it == h_g.cend() || it->second->children.empty())
      return {{-1, -1}, std::numeric_limits<type_weight>::max()};

    return it->second->children.begin()->get_value();
  }

  [[nodiscard]] H_out_map
  construct_h_out(
    std::vector<node_id_type> const &successors,
    type_collection_weights const   &sidetrack_distances) const // O(N+E*log(N))
  {
    H_out_map h_out;

    for (auto &node : graph.nodes) // O(N)
      h_out.insert(h_out.cend(), {node.get_id(), std::make_shared<H_out>()});

    for (auto const &edge_pair : sidetrack_distances) // O(E)
      {
        auto const &edge               = edge_pair.first;
        auto const &sidetrack_distance = edge_pair.second;

        auto const &tail = edge.first;
        auto const &succ = successors[tail];

        if (edge_pair.second != succ)
          {
            edge_info tmp;
            tmp.edge         = edge;
            tmp.delta_weight = sidetrack_distance;
            auto &children   = h_out[tail]->heap.children;

            children.insert(std::move(tmp)); // O(log(N))
          }
      }

    return h_out;
  }

  [[nodiscard]] H_out_map
  construct_h_out_linear(std::vector<node_id_type> const &successors,
                         type_collection_weights const   &sidetrack_distances,
                         std::size_t devices) const // O(N+E*log(N))
  {
    H_out_map  h_out;
    auto const num_nodes = graph.nodes.size();

    for (auto i = 0; i < num_nodes * devices; ++i)
      h_out.insert(h_out.cend(), {i, std::make_shared<H_out>()});

    for (auto const &edge_pair : sidetrack_distances) // O(E)
      {
        auto const &edge               = edge_pair.first;
        auto const &sidetrack_distance = edge_pair.second;

        auto const &tail = edge.first;
        auto const &succ = successors[tail];

        if (edge_pair.second != succ)
          {
            edge_info tmp;
            tmp.edge         = edge;
            tmp.delta_weight = sidetrack_distance;
            auto &children   = h_out[tail]->heap.children;

            children.insert(std::move(tmp)); // O(log(N))
          }
      }

    return h_out;
  }

  std::map<node_id_type, H_g_pointer>
  construct_h_g(std::map<node_id_type, H_out_pointer> const &h_out,
                std::vector<node_id_type> const &successors) // O(N*log(N))
  {
    std::map<node_id_type, H_g_pointer> res;

    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(graph.nodes.size());

    for (auto &node : graph.nodes) // O(N)
      {
        auto &tmp = successors[node.get_id()];

        if (tmp != node.get_id())
          sp_dependencies[tmp].insert(node.get_id());

        res.insert(res.cend(),
                   {node.get_id(), std::make_shared<H_g>()}); // O(1)
      }

    auto iterator = h_out.find(graph.nodes.size() - 1);
    if (iterator != h_out.cend() && !iterator->second->heap.children.empty())
      {
        res[graph.nodes.size() - 1]->children.emplace(
          iterator->second); // O(log(N))
      }

    std::queue<node_id_type> queue;

    for (auto it = ++graph.nodes.crbegin(); it != graph.nodes.crend();
         ++it) // O(N)
      if (successors[it->get_id()] == graph.nodes.size() - 1)
        queue.push(it->get_id());

    while (!queue.empty()) // O(N)
      {
        auto &deps = sp_dependencies[queue.front()];

        for (auto &n : deps)
          queue.push(n);

        auto &heap_node = res[queue.front()]; // O(log(N))

        iterator = h_out.find(queue.front());
        if (iterator != h_out.cend() &&
            !iterator->second->heap.children.empty())
          heap_node->children.emplace(iterator->second); // O(1)

        auto const &tmo = res[successors[queue.front()]];

        if (!tmo->children.empty())
          heap_node->children.emplace(tmo); // O(1)


        queue.pop();
      }

    return res;
  }

  std::map<node_id_type, H_g_pointer>
  construct_h_g_linear(std::map<node_id_type, H_out_pointer> const &h_out,
                       std::vector<node_id_type> const             &successors,
                       std::size_t devices) // O(N*log(N))
  {
    std::map<node_id_type, H_g_pointer> res;

    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(graph.nodes.size() * devices);

    auto const num_nodes = graph.nodes.size();

    for (auto i = 0; i < num_nodes * devices; ++i)
      res.insert(res.cend(), {i, std::make_shared<H_g>()});

    for (auto i = 0; i < num_nodes * devices; ++i) // O(N)
      {
        auto &tmp = successors[i];

        if (tmp != i)
          sp_dependencies[tmp].insert(i);
      }

    auto const last_index = num_nodes - 1;
    auto       iterator   = h_out.find(last_index);
    if (iterator != h_out.cend() && !iterator->second->heap.children.empty())
      {
        res[last_index]->children.emplace(iterator->second); // O(log(N))
      }

    std::queue<node_id_type> queue;

    for (auto i = last_index - 1; i >= 0; ++i)
      if (successors[i] == last_index)
        queue.push(i);

    while (!queue.empty()) // O(N)
      {
        auto &deps = sp_dependencies[queue.front()];

        for (auto &n : deps)
          queue.push(n);

        auto &heap_node = res[queue.front()]; // O(log(N))

        iterator = h_out.find(queue.front());
        if (iterator != h_out.cend() &&
            !iterator->second->heap.children.empty())
          heap_node->children.emplace(iterator->second); // O(1)

        auto const &tmo = res[successors[queue.front()]];

        if (!tmo->children.empty())
          heap_node->children.emplace(tmo); // O(1)


        queue.pop();
      }

    return res;
  }

  void
  get_g_edges_edges(std::map<edge_type, std::set<edge_type>> &edge_edges,
                    H_g_pointer const                        &h_g) const
  {
    std::size_t                                        j = 0;
    std::vector<std::set<H_g_content>::const_iterator> previous_steps;
    previous_steps.reserve(h_g->children.size());

    for (auto it = h_g->children.cbegin(); it != h_g->children.cend(); // O(N)
         ++it, ++j)
      {
        previous_steps.push_back(it);
        if (j > 0)
          {
            std::size_t external_parent = (j - 1) / 2;
            if (external_parent != j)
              {
                auto const parents =
                  previous_steps[external_parent]->get_edges();
                auto const children = it->get_edges(); // O(E)

                for (auto &parent : parents)
                  for (auto &child : children)
                    edge_edges[parent.edge].insert(child.edge);
              }
          }
      }
  }

  void
  get_edges_edges(std::map<edge_type, std::set<edge_type>>  &edge_edges,
                  std::map<node_id_type, H_g_pointer> const &h_gs) const
  {
    for (auto it = h_gs.begin(); it != h_gs.cend(); ++it) // O(N)
      {
        auto const &h_g_tm = *it;
        auto const &h_g    = h_g_tm.second;

        if (h_g)
          get_g_edges_edges(edge_edges, h_g);

        std::size_t j = 0;
        std::vector<typename std::set<H_g_content>::const_iterator>
          previous_external;

        for (auto it2 = h_g->children.begin(); it2 != h_g->children.cend();
             ++it2, ++j)
          {
            auto const &internal_content = *it2;
            previous_external.push_back(it2);

            if (j > 0)
              {
                std::size_t external_parent = (j - 1) / 2;
                if (external_parent != j)
                  {
                    auto edge_index =
                      previous_external[external_parent]->get_value().edge;
                    edge_edges[edge_index].insert(
                      internal_content.get_value().edge);
                  }
              }
          }
      }
  }


  [[nodiscard]] type_collection_weights
  sidetrack_distances(type_collection_weights const  &weights,
                      std::vector<type_weight> const &distances_from_sink) const
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
  

  [[nodiscard]] type_collection_weights
  sidetrack_distances(std::function<type_weight(edge_type const &)> &weights,
                      std::vector<type_weight> const &distances_from_sink) const
  {
    type_collection_weights res;
    auto const              num_nodes = graph.nodes.size();

    for (std::size_t i = 0; i < num_nodes; ++i)
      for (auto const &dep : graph.dependencies[i].second)
        {
          auto const tail = i;
          auto const head = dep;
          auto const edge = std::make_pair(tail, head);

          res.insert(res.cend(),
                     {edge,
                      weights(edge) + distances_from_sink[head] -
                        distances_from_sink[tail]}); // O(1)
        }

    return res;
  }

  [[nodiscard]] type_collection_weights
  sidetrack_distances_linear(
    type_collection_weights const  &weights,
    std::size_t                     devices,
    std::vector<type_weight> const &distances_from_sink) const
  {
    type_collection_weights res;
    auto const              num_nodes = graph.nodes.size();

    for (std::size_t s = 0; s < devices; ++s)
      for (std::size_t k = 0; k < devices; ++k)
        for (std::size_t i = 0; i < num_nodes; ++i)
          for (auto const &dep : graph.dependencies[i])
            {
              auto const tail = i + s * devices;
              auto const head = dep + k * devices;
              auto const edge = std::make_pair(tail, head);

              res.insert(res.cend(),
                         {edge,
                          weights[edge] + distances_from_sink[head] -
                            distances_from_sink[tail]}); // O(1)
            }

    return res;
  }


  [[nodiscard]] type_collection_weights
  sidetrack_distances_linear(
    std::function<type_weight(edge_type const &)> &weights,
    std::size_t                                    devices,
    std::vector<type_weight> const                &distances_from_sink) const
  {
    type_collection_weights res;
    auto const              num_nodes = graph.nodes.size();

    for (std::size_t s = 0; s < devices; ++s)
      for (std::size_t k = 0; k < devices; ++k)
        for (std::size_t i = 0; i < num_nodes; ++i)
          for (auto const &dep : graph.dependencies[i])
            {
              auto const tail = i + s * devices;
              auto const head = dep + k * devices;
              auto const edge = std::make_pair(tail, head);

              res.insert(res.cend(),
                         {edge,
                          weights(edge) + distances_from_sink[head] -
                            distances_from_sink[tail]}); // O(1)
            }

    return res;
  }

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


#endif // NETWORK_BUTCHER_KSP_H
