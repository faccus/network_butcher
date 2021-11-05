//
// Created by faccus on 01/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_H
#define NETWORK_BUTCHER_KEPPSTEIN_H

#include "ksp.h"

template <class T, typename id_content = io_id_type>
class KFinder_Eppstein : KFinder<T, id_content>
{
public:
  using path_info = typename KFinder<T, id_content>::path_info;
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

  using dij_res_type =
    std::pair<std::vector<node_id_type>, std::vector<type_weight>>;
  using H_out_map = std::map<node_id_type, H_out_pointer>;
  using base      = KFinder<T, id_content>;


  [[nodiscard]] std::vector<path_info>
  eppstein(type_collection_weights const &weights, std::size_t K)
  {
    std::function<type_weight(edge_type const &)> weight_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it != weights.cend())
          return it->second;
        return -1.;
      };

    return eppstein(weight_fun, K);
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
    std::function<type_weight(edge_type const &)> weight_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it != weights.cend())
          return it->second;
        return -1.;
      };

    return eppstein_linear(weight_fun, K, devices);
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


  std::vector<path_info>
  lazy_eppstein(type_collection_weights const &weights, std::size_t K)
  {
    std::function<type_weight(edge_type const &)> weights_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it != weights.cend())
          return it->second;
        return -1.;
      };

    return lazy_eppstein(weights_fun, K);
  }


  std::vector<path_info>
  lazy_eppstein(std::function<type_weight(edge_type const &)> &weights,
                std::size_t                                    K)
  {
    auto const &graph = base::graph;

    if (graph.nodes.empty() || K == 0)
      return {};

    auto const dij_res =
      base::shortest_path_tree(weights); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base::shortest_path_finder(dij_res, 0)};


    auto const epp_res = basic_lazy_eppstein(weights, K, dij_res);

    return helper_eppstein(dij_res, epp_res);
  }


  std::vector<path_info>
  lazy_eppstein_linear(type_collection_weights const &weights,
                       std::size_t                    K,
                       std::size_t                    devices)
  {
    std::function<type_weight(edge_type const &)> weights_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it != weights.cend())
          return it->second;
        return -1.;
      };

    return lazy_eppstein_linear(weights_fun, K, devices);
  }


  std::vector<path_info>
  lazy_eppstein_linear(std::function<type_weight(edge_type const &)> &weights,
                       std::size_t                                    K,

                       std::size_t devices)
  {
    auto const &graph = base::graph;

    if (graph.nodes.empty() || K == 0 || devices == 0)
      return {};
    if (devices == 1)
      return lazy_eppstein(weights, K);

    auto const dij_res = base::shortest_path_tree_linear(
      weights, devices); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base::shortest_path_finder(dij_res, 0)};


    auto const epp_res =
      basic_lazy_eppstein_linear(weights, K, dij_res, devices);

    return helper_eppstein(dij_res, epp_res);
  }


  explicit KFinder_Eppstein(Graph<T, id_content> const &g)
    : base(g){};

protected:
  [[nodiscard]] std::vector<path_info>
  helper_eppstein(dij_res_type const                    &dij_res,
                  std::vector<implicit_path_info> const &epp_res)
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


  [[nodiscard]] std::pair<bool, edge_info>
  side_track(node_id_type const                &j,
             std::map<node_id_type, H_g> const &h_g) const
  {
    auto const it = h_g.find(j);
    if (it == h_g.cend() || it->second.children.empty() ||
        (*it->second.children.begin())->heap.children.empty())
      return {false, {{-1, -1}, std::numeric_limits<type_weight>::max()}};

    return {true, *(*it->second.children.begin())->heap.children.begin()};
  }


  void
  get_internal_edges(std::map<edge_type, std::set<edge_type>> &edge_edges,
                     H_out_pointer const                      &h_out) const
  {
    std::size_t                                      j = 0;
    std::vector<std::set<edge_info>::const_iterator> previous_steps;
    previous_steps.reserve(h_out->heap.children.size());

    for (auto it = h_out->heap.children.cbegin();
         it != h_out->heap.children.cend();
         ++it, ++j)
      {
        previous_steps.push_back(it);

        std::size_t parent = j / 2;
        if (parent != j)
          {
            auto const &parent_edge  = previous_steps[parent]->edge;
            auto const &current_edge = it->edge;

            edge_edges[parent_edge].insert(current_edge);
          }
      }
  }

  void
  get_internal_edges(std::map<edge_type, std::set<edge_type>> &edge_edges,
                     H_g const                                &h_g,
                     bool include_h_outs = true) const
  {
    std::size_t                                          j = 0;
    std::vector<std::set<H_out_pointer>::const_iterator> previous_steps;
    previous_steps.reserve(h_g.children.size());
    const auto &graph = base::graph;

    for (auto it = h_g.children.cbegin(); it != h_g.children.cend(); ++it, ++j)
      {
        previous_steps.push_back(it);

        if (include_h_outs)
          get_internal_edges(edge_edges, *it);

        std::size_t parent = (j - 1) / 2;
        if (j > 0 && parent != j)
          {
            // for (auto i = 0; i <= parent; ++i)
            {
              auto const &parent_edge =
                (*previous_steps[parent])->heap.children.cbegin()->edge;
              auto const &child_edge = (*it)->heap.children.cbegin()->edge;

              edge_edges[parent_edge].insert(child_edge);
            }
          }
      }
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
    std::function<type_weight(edge_type const &)> &weights,
    std::size_t                                    devices,
    std::vector<type_weight> const                &distances_from_sink) const
  {
    type_collection_weights res;
    auto const             &graph     = base::graph;
    auto const              num_nodes = graph.nodes.size();

    for (std::size_t s = 0; s < devices; ++s)
      for (std::size_t k = 0; k < devices; ++k)
        for (std::size_t i = 1; i < num_nodes - 2; ++i)
          {
            node_id_type tail = i;

            if (s > 0)
              {
                if (s == 1)
                  tail += num_nodes - 1;
                else
                  tail += num_nodes - 1 + (s - 1) * (num_nodes - 2);
              }

            node_id_type head = i + 1;

            if (k > 0)
              {
                if (k == 1)
                  head += num_nodes - 1;
                else
                  head += num_nodes - 1 + (k - 1) * (num_nodes - 2);
              }

            auto const edge = std::make_pair(tail, head);

            res.insert(res.cend(),
                       {edge,
                        weights(edge) + distances_from_sink[head] -
                          distances_from_sink[tail]}); // O(1)
          }

    if (num_nodes > 1)
      for (std::size_t k = 0; k < devices; ++k)
        {
          const node_id_type tail = 0;
          node_id_type       head = 1;

          if (k > 0)
            {
              if (k == 1)
                head += num_nodes - 1;
              else
                head += num_nodes - 1 + (k - 1) * (num_nodes - 2);
            }

          auto const edge = std::make_pair(tail, head);

          res.insert(res.cend(),
                     {edge,
                      weights(edge) + distances_from_sink[head] -
                        distances_from_sink[tail]}); // O(1)
        }

    if (num_nodes > 2)
      for (std::size_t s = 0; s < devices; ++s)
        {
          node_id_type       tail = num_nodes - 2;
          const node_id_type head = num_nodes - 1;

          if (s > 0)
            {
              if (s == 1)
                tail += num_nodes - 1;
              else
                tail += num_nodes - 1 + (s - 1) * (num_nodes - 2);
            }

          auto const edge = std::make_pair(tail, head);

          res.insert(res.cend(),
                     {edge,
                      weights(edge) + distances_from_sink[head] -
                        distances_from_sink[tail]}); // O(1)
        }

    return res;
  }


private:
  [[nodiscard]] std::map<edge_type, std::set<edge_type>>
  get_edge_edges(std::map<node_id_type, H_g> const &h_gs) const
  {
    std::map<edge_type, std::set<edge_type>> res;
    for (auto it = h_gs.cbegin(); it != h_gs.cend(); ++it) // O(N)
      get_internal_edges(res, it->second);
    return res;
  }


  [[nodiscard]] H_out_map
  helper_construct_h_out(std::vector<node_id_type> const &successors,
                         type_collection_weights const   &sidetrack_distances,
                         auto const                       real_num_nodes) const
  {
    H_out_map   h_out;
    auto const &graph     = base::graph;
    auto const  num_nodes = graph.nodes.size();

    for (auto i = 0; i < real_num_nodes; ++i)
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

  [[nodiscard]] H_out_map
  construct_h_out(
    std::vector<node_id_type> const &successors,
    type_collection_weights const   &sidetrack_distances) const // O(N+E*log(N))
  {
    return helper_construct_h_out(successors,
                                  sidetrack_distances,
                                  base::graph.nodes.size());
  }

  [[nodiscard]] H_out_map
  construct_h_out_linear(std::vector<node_id_type> const &successors,
                         type_collection_weights const   &sidetrack_distances,
                         std::size_t devices) const // O(N+E*log(N))
  {
    auto const num_nodes      = base::graph.nodes.size();
    auto const real_num_nodes = num_nodes + (num_nodes - 2) * (devices - 1);

    return helper_construct_h_out(successors,
                                  sidetrack_distances,
                                  real_num_nodes);
  }


  [[nodiscard]] std::map<node_id_type, H_g>
  helper_construct_h_g(std::map<node_id_type, H_out_pointer> const &h_out,
                       std::vector<node_id_type> const             &successors,
                       std::size_t const &num_nodes) const // O(N*log(N))
  {
    std::map<node_id_type, H_g> res;

    auto const &graph = base::graph;
    auto const &nodes = graph.nodes;


    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(num_nodes);

    for (auto i = 0; i < num_nodes; ++i)
      res.insert(res.cend(), {i, H_g()});

    for (auto i = 0; i < num_nodes; ++i) // O(N)
      {
        auto const &tmp = successors[i];

        if (tmp != i)
          sp_dependencies[tmp].insert(i);
      }

    auto const sink = nodes.size() - 1;

    auto iterator = h_out.find(sink);
    if (iterator != h_out.cend() && !iterator->second->heap.children.empty())
      {
        res[sink].children.emplace(iterator->second); // O(log(N))
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
          heap_node.children.emplace(iterator->second); // O(1)

        auto const &tmo = res[successors[queue.front()]];

        if (!tmo.children.empty())
          heap_node.children.insert(tmo.children.begin(),
                                    tmo.children.end()); // O(1)


        queue.pop();
      }

    return res;
  }

  [[nodiscard]] std::map<node_id_type, H_g>
  construct_h_g(
    std::map<node_id_type, H_out_pointer> const &h_out,
    std::vector<node_id_type> const &successors) const // O(N*log(N))
  {
    return helper_construct_h_g(h_out, successors, base::graph.nodes.size());
  }

  std::map<node_id_type, H_g>
  construct_h_g_linear(std::map<node_id_type, H_out_pointer> const &h_out,
                       std::vector<node_id_type> const             &successors,
                       std::size_t devices) // O(N*log(N))
  {
    auto const num_nodes = base::graph.nodes.size();
    return helper_construct_h_g(h_out,
                                successors,
                                num_nodes + (num_nodes - 2) * (devices - 1));
  }


  H_out_map::iterator
  construct_partial_h_out(H_out_map                       &h_out,
                          type_collection_weights const   &sidetrack_distances,
                          std::vector<node_id_type> const &successors,
                          node_id_type                     node) const
  {
    {
      auto it = h_out.find(node);
      if (it != h_out.cend())
        return it;
    }

    auto const &graph = base::graph;

    auto       it   = h_out.emplace(node, std::make_shared<H_out>());
    auto const succ = successors[node];

    for (auto const &exit : graph.dependencies[node].second)
      if (exit != succ)
        {
          auto const edge    = std::make_pair(node, exit);
          auto const it_dist = sidetrack_distances.find(edge);

          if (it_dist == sidetrack_distances.cend())
            {
              continue;
            }

          edge_info tmp;
          tmp.edge         = edge;
          tmp.delta_weight = it_dist->second;
          auto &children   = h_out[node]->heap.children;

          children.insert(std::move(tmp)); // O(log(N))
        }

    return it.first;
  }

  H_out_map::iterator
  construct_partial_h_out_linear(
    H_out_map                       &h_out,
    type_collection_weights const   &sidetrack_distances,
    std::vector<node_id_type> const &successors,
    node_id_type                     node,
    std::size_t                      devices) const
  {
    {
      auto it = h_out.find(node);
      if (it != h_out.cend())
        return it;
    }

    auto const &graph = base::graph;
    auto        it    = h_out.emplace(node, std::make_shared<H_out>());
    auto const  succ  = successors[node];

    for (auto const &exit :
         graph
           .dependencies[node < graph.nodes.size() ?
                           node :
                           (node - 2) % (graph.nodes.size() - 2) + 1]
           .second)
      {
        if (exit != graph.nodes.size() - 1)
          {
            for (std::size_t j = 0; j < devices; ++j)
              {
                auto const head = j == 0 ? exit :
                                           exit + graph.nodes.size() - 1 +
                                             (j - 1) * (graph.nodes.size() - 2);
                if (head != succ)
                  {
                    auto const edge    = std::make_pair(node, head);
                    auto const it_dist = sidetrack_distances.find(edge);

                    if (it_dist == sidetrack_distances.cend())
                      continue;

                    edge_info tmp;
                    tmp.edge         = edge;
                    tmp.delta_weight = it_dist->second;
                    auto &children   = h_out[node]->heap.children;

                    children.insert(std::move(tmp)); // O(log(N))
                  }
              }
          }
      }

    return it.first;
  }


  std::map<node_id_type, H_g>::iterator
  construct_partial_h_g(std::map<node_id_type, H_g>           &h_g,
                        std::map<node_id_type, H_out_pointer> &h_out,
                        type_collection_weights const   &sidetrack_distances,
                        std::vector<node_id_type> const &successors,
                        node_id_type                     node,
                        std::map<edge_type, std::set<edge_type>> &edge_edges)
  {
    {
      auto it = h_g.find(node);
      if (it != h_g.cend())
        return it;
    }

    auto const &graph = base::graph;

    {
      if (node == graph.nodes.size() - 1)
        {
          auto it   = h_g.emplace(node, H_g());
          auto it_2 = construct_partial_h_out(h_out,
                                              sidetrack_distances,
                                              successors,
                                              node);

          if (!it_2->second->heap.children.empty())
            {
              it.first->second.children.insert(it_2->second);
              get_internal_edges(edge_edges, it.first->second);
            }

          return it.first;
        }
    }

    auto const successor = successors[node];

    auto it_previous = construct_partial_h_g(
      h_g, h_out, sidetrack_distances, successors, successor, edge_edges);
    auto it = h_g.emplace(node, it_previous->second);
    auto it_2 =
      construct_partial_h_out(h_out, sidetrack_distances, successors, node);


    if (!it_2->second->heap.children.empty())
      {
        it.first->second.children.insert(it_2->second);
        get_internal_edges(edge_edges, it_2->second);
      }

    get_internal_edges(edge_edges, it.first->second, false);

    return it.first;
  }

  std::map<node_id_type, H_g>::iterator
  construct_partial_h_g_linear(
    std::map<node_id_type, H_g>              &h_g,
    std::map<node_id_type, H_out_pointer>    &h_out,
    type_collection_weights const            &sidetrack_distances,
    std::vector<node_id_type> const          &successors,
    node_id_type                              node,
    std::size_t                               devices,
    std::map<edge_type, std::set<edge_type>> &edge_edges)
  {
    {
      auto it = h_g.find(node);
      if (it != h_g.cend())
        return it;
    }

    auto const &graph = base::graph;

    {
      if (node == graph.nodes.size() - 1)
        {
          auto it   = h_g.emplace(node, H_g());
          auto it_2 = construct_partial_h_out_linear(
            h_out, sidetrack_distances, successors, node, devices);

          if (!it_2->second->heap.children.empty())
            {
              it.first->second.children.insert(it_2->second);
              get_internal_edges(edge_edges, it.first->second);
            }

          return it.first;
        }
    }

    auto const successor = successors[node];

    auto it_previous = construct_partial_h_g_linear(h_g,
                                                    h_out,
                                                    sidetrack_distances,
                                                    successors,
                                                    successor,
                                                    devices,
                                                    edge_edges);
    auto it          = h_g.emplace(node, it_previous->second);
    auto it2         = construct_partial_h_out_linear(
              h_out, sidetrack_distances, successors, node, devices);

    if (!it2->second->heap.children.empty())
      {
        it.first->second.children.insert(it2->second);
        get_internal_edges(edge_edges, it2->second);
      }

    get_internal_edges(edge_edges, it.first->second, false);

    return it.first;
  }


  std::vector<implicit_path_info>
  base_path_selector_eppstein(
    std::size_t                                     K,
    dij_res_type const                             &dij_res,
    type_collection_weights const                  &sidetrack_distances_res,
    std::map<node_id_type, H_g> const              &h_g,
    std::map<edge_type, std::set<edge_type>> const &edges_edges) const
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()});

    auto const &successors          = dij_res.first;
    auto const &graph               = base::graph;

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

        if (f_res.first)
          {
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

            for (auto const &f : it->second)
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

                if (!SK.sidetracks.empty())
                  {
                    auto n =
                      mod_sk.sidetracks[mod_sk.sidetracks.size() - 2].second;
                    while (n != graph.nodes.size() - 1 &&
                           n != mod_sk.sidetracks.back().first)
                      {
                        n = successors[n];
                      }

                    if (n != mod_sk.sidetracks.back().first)
                      continue;
                  }

                Q.insert(std::move(mod_sk));
              }
          }
      }

    return res;
  }


  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein(std::function<type_weight(edge_type const &)> &weights,
                 std::size_t                                    K,
                 dij_res_type const                            &dij_res)
  {
    auto const &graph = base::graph;

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return {{{}, dij_res.second.front()}};

    auto const sidetrack_distances_res =
      sidetrack_distances(weights, dij_res.second);                    // O(E)
    auto const shortest_path = base::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out = construct_h_out(successors, sidetrack_distances_res);

    auto const h_g         = construct_h_g(h_out, successors);
    auto       edges_edges = get_edge_edges(h_g);

    return base_path_selector_eppstein(
      K, dij_res, sidetrack_distances_res, h_g, edges_edges);
  }

  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein_linear(std::function<type_weight(edge_type const &)> &weights,
                        std::size_t                                    K,
                        std::size_t                                    devices,
                        dij_res_type const                            &dij_res)
  {
    auto const &graph = base::graph;

    if (graph.nodes.empty() || devices == 0)
      return {};
    else if (K == 1)
      return {{{}, dij_res.second.front()}};
    else if (devices == 1)
      return basic_eppstein(weights, K, dij_res);

    auto const sidetrack_distances_res =
      sidetrack_distances_linear(weights, devices, dij_res.second);    // O(E)
    auto const shortest_path = base::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors = dij_res.first;

    auto const h_out =
      construct_h_out_linear(successors, sidetrack_distances_res, devices);

    auto const h_g         = construct_h_g_linear(h_out, successors, devices);
    auto       edges_edges = get_edge_edges(h_g);

    return base_path_selector_eppstein(
      K, dij_res, sidetrack_distances_res, h_g, edges_edges);
  }


  [[nodiscard]] std::vector<implicit_path_info>
  basic_lazy_eppstein(std::function<type_weight(edge_type const &)> &weights,
                      std::size_t                                    K,
                      dij_res_type const                            &dij_res)
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

    H_out_map                   h_out;
    std::map<node_id_type, H_g> h_g;

    auto edges_edges = std::map<edge_type, std::set<edge_type>>();


    construct_partial_h_g(
      h_g, h_out, sidetrack_distances_res, successors, 0, edges_edges);

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

        construct_partial_h_g(h_g,
                              h_out,
                              sidetrack_distances_res,
                              successors,
                              e.second,
                              edges_edges);


        auto const f_res = side_track(e.second, h_g);

        if (f_res.first)
          {
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

                if (!SK.sidetracks.empty())
                  {
                    auto n =
                      mod_sk.sidetracks[mod_sk.sidetracks.size() - 2].second;
                    while (n != graph.nodes.size() - 1 &&
                           n != mod_sk.sidetracks.back().first)
                      {
                        n = successors[n];
                      }

                    if (n != mod_sk.sidetracks.back().first)
                      continue;
                  }

                Q.insert(std::move(mod_sk));
              }
          }
      }


    return res;
  }

  [[nodiscard]] std::vector<implicit_path_info>
  basic_lazy_eppstein_linear(
    std::function<type_weight(edge_type const &)> &weights,
    std::size_t                                    K,
    dij_res_type const                            &dij_res,
    std::size_t                                    devices)
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(K)

    auto const &graph = base::graph;

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return res;


    auto const sidetrack_distances_res =
      sidetrack_distances_linear(weights, devices, dij_res.second);    // O(E)
    auto const shortest_path = base::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors = dij_res.first;

    H_out_map                   h_out;
    std::map<node_id_type, H_g> h_g;

    auto edges_edges = std::map<edge_type, std::set<edge_type>>();


    construct_partial_h_g_linear(
      h_g, h_out, sidetrack_distances_res, successors, 0, devices, edges_edges);

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

        construct_partial_h_g_linear(h_g,
                                     h_out,
                                     sidetrack_distances_res,
                                     successors,
                                     e.second,
                                     devices,
                                     edges_edges);


        auto const f_res = side_track(e.second, h_g);

        if (f_res.first)
          {
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

                if (!SK.sidetracks.empty())
                  {
                    auto n =
                      mod_sk.sidetracks[mod_sk.sidetracks.size() - 2].second;
                    while (n != graph.nodes.size() - 1 &&
                           n != mod_sk.sidetracks.back().first)
                      {
                        n = successors[n];
                      }

                    if (n != mod_sk.sidetracks.back().first)
                      continue;
                  }

                Q.insert(std::move(mod_sk));
              }
          }
      }


    return res;
  }
};


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
