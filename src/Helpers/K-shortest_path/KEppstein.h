//
// Created by faccus on 01/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_H
#define NETWORK_BUTCHER_KEPPSTEIN_H

#include "ksp.h"

template <class T, typename id_content = io_id_type>
class KFinder_Eppstein : KFinder<T, id_content>
{
protected:
  using base = KFinder<T, id_content>;

public:
  using path_info = typename base::path_info;
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


  [[nodiscard]] std::vector<path_info>
  eppstein(type_collection_weights const &weights, std::size_t K)
  {
    auto const &graph = base::graph;

    if (graph.nodes.empty() || K == 0)
      return {};

    auto const dij_res =
      base::shortest_path_tree(weights); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base::shortest_path_finder(dij_res, 0)};

    auto const epp_res = basic_eppstein(weights, K, dij_res);

    return helper_eppstein(dij_res, epp_res);
  }

  [[nodiscard]] std::vector<path_info>
  eppstein(std::function<type_weight(edge_type const &)> &weights,
           std::size_t                                    K)
  {
    auto const &graph = base::graph;

    if (graph.nodes.empty() || K == 0)
      return {};

    auto const dij_res =
      base::shortest_path_tree(weights); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base::shortest_path_finder(dij_res, 0)};


    auto const epp_res = basic_eppstein(weights, K, dij_res);

    return helper_eppstein(dij_res, epp_res);
  }


  [[nodiscard]] std::vector<path_info>
  eppstein_linear(type_collection_weights const &weights,
                  std::size_t                    K,
                  std::size_t                    devices)
  {
    auto const &graph = base::graph;

    if (graph.nodes.empty() || K == 0 || devices == 0)
      return {};
    if (devices == 1)
      return eppstein(weights, K);

    auto const dij_res = base::shortest_path_tree_linear(
      weights, devices); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base::shortest_path_finder(dij_res, 0)};


    auto const epp_res = basic_eppstein_linear(weights, K, devices, dij_res);

    return helper_eppstein(dij_res, epp_res);
  }

  [[nodiscard]] std::vector<path_info>
  eppstein_linear(std::function<type_weight(edge_type const &)> &weights,
                  std::size_t                                    K,
                  std::size_t                                    devices)
  {
    auto const &graph = base::graph;

    if (graph.nodes.empty() || K == 0 || devices == 0)
      return {};
    if (devices == 1)
      return eppstein(weights, K);

    auto const dij_res = base::shortest_path_tree_linear(
      weights, devices); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base::shortest_path_finder(dij_res, 0)};


    auto const epp_res = basic_eppstein_linear(weights, K, devices, dij_res);

    return helper_eppstein(dij_res, epp_res);
  }


  explicit KFinder_Eppstein(Graph<T, id_content> const &g)
    : base(g){};

protected:
  using H_out_map = std::map<node_id_type, H_out_pointer>;


  std::pair<bool, edge_info>
  side_track(node_id_type const                        &j,
             std::map<node_id_type, H_g_pointer> const &h_g)
  {
    auto const it = h_g.find(j);
    if (it == h_g.cend() || it->second->children.empty())
      return {false, {{-1, -1}, std::numeric_limits<type_weight>::max()}};

    return {true, it->second->children.begin()->get_value()};
  }


  [[nodiscard]] H_out_map
  construct_h_out(
    std::vector<node_id_type> const &successors,
    type_collection_weights const   &sidetrack_distances) const // O(N+E*log(N))
  {
    H_out_map   h_out;
    auto const &graph = base::graph;

    for (auto &node : graph.nodes) // O(N)
      h_out.insert(h_out.cend(), {node.get_id(), std::make_shared<H_out>()});

    for (auto const &edge_pair : sidetrack_distances) // O(E)
      {
        auto const &edge               = edge_pair.first;
        auto const &sidetrack_distance = edge_pair.second;

        auto const &tail = edge.first;
        auto const &succ = successors[tail];

        if (edge.second != succ)
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
    H_out_map   h_out;
    auto const &graph     = base::graph;
    auto const  num_nodes = graph.nodes.size();

    for (auto i = 0; i < num_nodes * devices; ++i)
      h_out.insert(h_out.cend(), {i, std::make_shared<H_out>()});

    for (auto const &edge_pair : sidetrack_distances) // O(E)
      {
        auto const &edge               = edge_pair.first;
        auto const &sidetrack_distance = edge_pair.second;

        auto const &tail = edge.first;
        auto const &succ = successors[tail];

        if (edge.second != succ)
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

    auto const &graph     = base::graph;
    auto const  num_nodes = graph.nodes.size();
    auto const &nodes     = graph.nodes;

    sp_dependencies.resize(num_nodes);

    for (auto &node : nodes) // O(N)
      {
        auto &tmp = successors[node.get_id()];

        if (tmp != node.get_id())
          sp_dependencies[tmp].insert(node.get_id());

        res.insert(res.cend(),
                   {node.get_id(), std::make_shared<H_g>()}); // O(1)
      }

    auto iterator = h_out.find(num_nodes - 1);
    if (iterator != h_out.cend() && !iterator->second->heap.children.empty())
      {
        res[num_nodes - 1]->children.emplace(iterator->second); // O(log(N))
      }

    std::queue<node_id_type> queue;

    for (auto it = ++nodes.crbegin(); it != nodes.crend(); ++it) // O(N)
      if (successors[it->get_id()] == num_nodes - 1)
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

    auto const &graph     = base::graph;
    auto const  num_nodes = graph.nodes.size();
    auto const &nodes     = graph.nodes;

    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(num_nodes * devices);

    for (auto i = 0; i < num_nodes * devices; ++i)
      res.insert(res.cend(), {i, std::make_shared<H_g>()});

    for (auto i = 0; i < num_nodes * devices; ++i) // O(N)
      {
        auto const &tmp = successors[i];

        if (tmp != i)
          sp_dependencies[tmp].insert(i);
      }

    auto const sink = num_nodes - 1;

    auto iterator = h_out.find(sink);
    if (iterator != h_out.cend() && !iterator->second->heap.children.empty())
      {
        res[sink]->children.emplace(iterator->second); // O(log(N))
      }

    std::queue<node_id_type> queue;

    for (auto i = 0; i < res.size(); ++i)
      if (successors[i] == sink)
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
    for (auto it = h_gs.cbegin(); it != h_gs.cend(); ++it) // O(N)
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
    auto const             &graph     = base::graph;
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
    auto const             &graph     = base::graph;
    auto const              num_nodes = graph.nodes.size();

    for (std::size_t s = 0; s < devices; ++s)
      for (std::size_t k = 0; k < devices; ++k)
        for (std::size_t i = 0; i < num_nodes; ++i)
          for (auto const &dep : graph.dependencies[i].second)
            {
              auto const tail = i + s * num_nodes;
              auto const head = dep + k * num_nodes;
              auto const edge = std::make_pair(tail, head);

              auto const it = weights.find(edge);
              if (it == weights.cend())
                {
                  std::cout << "Error: missing weight (" << tail << ", " << head
                            << ")" << std::endl;
                }
              else
                res.insert(res.cend(),
                           {edge,
                            it->second + distances_from_sink[head] -
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
    auto const             &graph     = base::graph;
    auto const              num_nodes = graph.nodes.size();

    for (std::size_t s = 0; s < devices; ++s)
      for (std::size_t k = 0; k < devices; ++k)
        for (std::size_t i = 0; i < num_nodes; ++i)
          for (auto const &dep : graph.dependencies[i].second)
            {
              auto const tail = i + s * num_nodes;
              auto const head = dep + k * num_nodes;
              auto const edge = std::make_pair(tail, head);

              res.insert(res.cend(),
                         {edge,
                          weights(edge) + distances_from_sink[head] -
                            distances_from_sink[tail]}); // O(1)
            }

    return res;
  }


  [[nodiscard]] std::vector<path_info>
  helper_eppstein(
    std::pair<std::vector<node_id_type>, std::vector<type_weight>> const
                                                            &dij_res,
    std::vector<KFinder_Eppstein::implicit_path_info> const &epp_res)
  {
    std::vector<path_info> res;
    auto const            &graph = base::graph;

    for (auto implicit_path = epp_res.cbegin(); implicit_path != epp_res.cend();
         ++implicit_path)
      {
        path_info info;
        info.length = implicit_path->length;
        info.path.reserve(graph.nodes.size());

        auto const &sidetracks = implicit_path->sidetracks;

        auto        it  = sidetracks.cbegin();
        std::size_t ind = 0;

        while (ind != graph.nodes.back().get_id())
          {
            info.path.push_back(ind);
            if (it != sidetracks.cend() && it->first == ind)
              {
                ind = it->second;
                ++it;
              }
            else
              ind = dij_res.first[ind];
          }

        info.path.push_back(ind);
        res.emplace_back(std::move(info));
      }

    return res;
  }


  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein(type_collection_weights const             &weights,
                 std::size_t                                K,
                 std::pair<std::vector<node_id_type>,
                           std::vector<type_weight>> const &dij_res)
  {
    std::function<type_weight(edge_type const &)> &weights_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it == weights.cend())
          return -1.;
        return it->second;
      };

    return basic_eppstein(weights_fun, K, dij_res);
  }


  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein_linear(type_collection_weights const             &weights,
                        std::size_t                                K,
                        std::size_t                                devices,
                        std::pair<std::vector<node_id_type>,
                                  std::vector<type_weight>> const &dij_res)
  {
    std::function<type_weight(edge_type const &)> &weights_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it == weights.cend())
          return -1.;
        return it->second;
      };

    return basic_eppstein_linear(weights_fun, K, devices, dij_res);
  }

  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein(std::function<type_weight(edge_type const &)> &weights,
                 std::size_t                                    K,
                 std::pair<std::vector<node_id_type>,
                           std::vector<type_weight>> const     &dij_res)
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(K)

    auto const &graph = base::graph;

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return res;


    auto const sidetrack_distances_res =
      sidetrack_distances(weights, dij_res.second);                    // O(E)
    auto const shortest_path = base::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out = construct_h_out(successors, sidetrack_distances_res);

    auto const h_g         = construct_h_g(h_out, successors);
    auto       edges_edges = std::map<edge_type, std::set<edge_type>>();
    get_edges_edges(edges_edges, h_g);

    auto const first_side_track_res = side_track(0, h_g);
    if (!first_side_track_res.first)
      return res;
    auto const &first_side_track = first_side_track_res.second;

    std::set<implicit_path_info> Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.insert(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = *Q.begin();
        Q.erase(Q.begin());
        res.push_back(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
            continue;
          }

        {
          auto const f_res = side_track(e.second, h_g);
          if (!f_res.first)
            continue;
          auto const &f = f_res.second;

          auto mod_sk = SK;
          mod_sk.sidetracks.push_back(f.edge);
          mod_sk.length += f.delta_weight;
          Q.insert(std::move(mod_sk));
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
                    continue;
                  }

                auto mod_sk = SK;
                mod_sk.sidetracks.push_back(f);
                mod_sk.length += (ut->second - ot->second);

                Q.insert(std::move(mod_sk));
              }
          }
      }


    return res;
  }


  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein_linear(std::function<type_weight(edge_type const &)> &weights,
                        std::size_t                                    K,
                        std::size_t                                    devices,
                        std::pair<std::vector<node_id_type>,
                                  std::vector<type_weight>> const     &dij_res)
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(K)
    auto const &graph = base::graph;

    if (graph.nodes.empty() || devices == 0)
      return {};
    else if (K == 1)
      return res;
    else if (devices == 1)
      return basic_eppstein(weights, K, dij_res);


    auto const sidetrack_distances_res =
      sidetrack_distances_linear(weights, devices, dij_res.second);    // O(E)
    auto const shortest_path = base::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out =
      construct_h_out_linear(successors, sidetrack_distances_res, devices);

    auto const h_g         = construct_h_g_linear(h_out, successors, devices);
    auto       edges_edges = std::map<edge_type, std::set<edge_type>>();
    get_edges_edges(edges_edges, h_g);

    auto const first_side_track_res = side_track(0, h_g);
    if (!first_side_track_res.first)
      return res;
    auto const &first_side_track = first_side_track_res.second;

    std::set<implicit_path_info> Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.insert(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = *Q.begin();
        Q.erase(Q.begin());
        res.push_back(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
            continue;
          }


        auto const f_res = side_track(e.second, h_g);

        if (!f_res.first)
          continue;

        auto const &f = f_res.second;
        auto const  fake_node =
          (f.edge.first != 0 && f.edge.first % graph.nodes.size() == 0) ||
          (f.edge.second != graph.nodes.size() - 1 &&
           f.edge.second % graph.nodes.size() == graph.nodes.size() - 1);

        if (fake_node)
          continue;

        {
          auto mod_sk = SK;
          mod_sk.sidetracks.push_back(f.edge);
          mod_sk.length += f.delta_weight;
          Q.insert(std::move(mod_sk));
        }


        auto const it = edges_edges.find(e);
        if (it != edges_edges.cend())
          {
            SK.sidetracks.pop_back();

            for (auto &f_child : it->second)
              {
                auto const fake_node =
                  (f_child.first != 0 &&
                   f_child.first % graph.nodes.size() == 0) ||
                  (f_child.second != graph.nodes.size() - 1 &&
                   f_child.second % graph.nodes.size() ==
                     graph.nodes.size() - 1);

                if (fake_node)
                  continue;

                auto ut = sidetrack_distances_res.find(f_child);

                if (ut == sidetrack_distances_res.cend())
                  {
                    std::cout << "Error: cannot find proper sidetrack distance "
                                 "for edge ("
                              << f_child.first << ", " << f_child.second << ")"
                              << std::endl;
                    continue;
                  }

                auto mod_sk = SK;
                mod_sk.sidetracks.push_back(f_child);
                mod_sk.length += (ut->second - ot->second);

                Q.insert(std::move(mod_sk));
              }
          }
      }


    return res;
  }


  void
  build_h_g(std::map<node_id_type, H_g_pointer> &h_g, node_id_type node_id)
  {}


  [[nodiscard]] std::vector<implicit_path_info>
  basic_lazy_eppstein(std::function<type_weight(edge_type const &)> &weights,
                      std::size_t                                    K,
                      std::pair<std::vector<node_id_type>,
                                std::vector<type_weight>> const     &dij_res)
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(K)

    auto const &graph = base::graph;

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return res;


    auto const sidetrack_distances_res =
      sidetrack_distances(weights, dij_res.second);                    // O(E)
    auto const shortest_path = base::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out = construct_h_out(successors, sidetrack_distances_res);

    std::map<node_id_type, H_g_pointer> const h_g =
      construct_h_g(h_out, successors);
    auto edges_edges = std::map<edge_type, std::set<edge_type>>();
    get_edges_edges(edges_edges, h_g);

    auto const first_side_track_res = side_track(0, h_g);
    if (!first_side_track_res.first)
      return res;
    auto const &first_side_track = first_side_track_res.second;

    std::set<implicit_path_info> Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.insert(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = *Q.begin();
        Q.erase(Q.begin());
        res.push_back(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
            continue;
          }

        {
          auto const f_res = side_track(e.second, h_g);
          if (!f_res.first)
            continue;
          auto const &f = f_res.second;

          auto mod_sk = SK;
          mod_sk.sidetracks.push_back(f.edge);
          mod_sk.length += f.delta_weight;
          Q.insert(std::move(mod_sk));
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
                    continue;
                  }

                auto mod_sk = SK;
                mod_sk.sidetracks.push_back(f);
                mod_sk.length += (ut->second - ot->second);

                Q.insert(std::move(mod_sk));
              }
          }
      }


    return res;
  }
};


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
