//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
#define NETWORK_BUTCHER_KEPPSTEIN_LAZY_H

#include "KEppstein.h"

template <class T, typename id_content = io_id_type>
class KFinder_Lazy_Eppstein : public KFinder<T, id_content>
{
public:
  using base          = KFinder<T, id_content>;
  using base_shortest = Shortest_path_finder<T, id_content>;

  using H_out_map = std::map<node_id_type, H_out_pointer>;


  /// Applies the lazy Eppstein algorithm to find the k-shortest paths on the
  /// given graph (from the first node to the last one)
  /// \param weights The weights associated to the different edges
  /// \param K The number of shortest paths to find
  /// \return The shortest paths
  [[nodiscard]] std::vector<path_info>
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


  /// Applies the lazy Eppstein algorithm to find the k-shortest paths on the
  /// given graph (from the first node to the last one)
  /// \param weights The weights associated to the different edges
  /// \param K The number of shortest paths to find
  /// \return The shortest paths
  [[nodiscard]] std::vector<path_info>
  lazy_eppstein(std::function<type_weight(edge_type const &)> &weights,
                std::size_t                                    K)
  {
    auto const &graph = base_shortest::graph;

    if (graph.nodes.empty() || K == 0)
      return {};

    auto const dij_res = base_shortest::shortest_path_tree(
      weights); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base_shortest::shortest_path_finder(dij_res, 0)};


    auto const epp_res = basic_lazy_eppstein(weights, K, dij_res);

    return base::helper_eppstein(dij_res, epp_res);
  }

  explicit KFinder_Lazy_Eppstein(Graph<T, id_content> const &g)
    : base(g){};

  virtual ~KFinder_Lazy_Eppstein() = default;

private:
  std::pair<bool, std::map<node_id_type, H_g>::iterator>
  find_h_g_in_map(std::map<node_id_type, H_g> &h_g, node_id_type node)
  {
    auto it = h_g.find(node);
    return {it != h_g.end(), it};
  }


  /// It will add to the h_out map the h_out associates to the current node
  /// \param h_out The h_out map
  /// \param sidetrack_distances The sidetrack distances
  /// \param successors The successor collection
  /// \param node The node associated to the h_out to construct
  /// \return The iterator of the added h_out
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

    auto const &graph = base_shortest::graph;

    auto it = h_out.emplace(node, std::make_shared<H_out<edge_info>>());
    it.first->second->heap.id = node;

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

          edge_info tmp(edge, it_dist->second);
          auto     &children = h_out[node]->heap.children;

          children.insert(std::move(tmp)); // O(log(N))
        }

    return it.first;
  }

  /// It will add to the h_g map the h_g associated to the current node. It will
  /// also update the edge_edges map (that associated every edge to its
  /// children)
  /// \param h_g The h_g map
  /// \param h_out The h_out map
  /// \param sidetrack_distances The sidetrack distances
  /// \param successors The successor collection
  /// \param node The node associated to the h_out to construct
  /// \param edge_edges The edge_edges map
  /// \return The iterator to the added element
  std::map<node_id_type, H_g>::iterator
  construct_partial_h_g(
    std::map<node_id_type, H_g>           &h_g,
    std::map<node_id_type, H_out_pointer> &h_out,
    type_collection_weights const         &sidetrack_distances,
    std::vector<node_id_type> const       &successors,
    node_id_type                           node,
    std::map<std::pair<edge_pointer, node_id_type>,
             std::set<std::pair<edge_pointer, node_id_type>>> &edge_edges)
  {
    auto pair_iterator = find_h_g_in_map(h_g, node);
    if (pair_iterator.first)
      return pair_iterator.second;

    auto const &graph = base_shortest::graph;


    if (node == graph.nodes.size() - 1)
      {
        auto inserted_h_g = h_g.emplace(node, H_g()).first;

        auto to_insert_h_out =
          construct_partial_h_out(h_out, sidetrack_distances, successors, node);

        if (!to_insert_h_out->second->heap.children.empty())
          {
            inserted_h_g->second.children.insert(to_insert_h_out->second);
            base::get_internal_edges(edge_edges,
                                     inserted_h_g->second,
                                     inserted_h_g->second.id);
          }

        return inserted_h_g;
      }

    auto const successor = successors[node];

    auto previous_inserted_h_g = construct_partial_h_g(
      h_g, h_out, sidetrack_distances, successors, successor, edge_edges);

    auto inserted_h_g = h_g.emplace(node, previous_inserted_h_g->second);
    inserted_h_g.first->second.id = node;

    auto current_node_h_out =
      construct_partial_h_out(h_out, sidetrack_distances, successors, node);


    if (!current_node_h_out->second->heap.children.empty())
      {
        inserted_h_g.first->second.children.insert(current_node_h_out->second);
        base::get_internal_edges(edge_edges,
                                 current_node_h_out->second,
                                 inserted_h_g.first->second.id);
      }

    base::get_internal_edges(edge_edges, inserted_h_g.first->second);

    return inserted_h_g.first;
  }

  /// The basic function for the lazy Eppstein algorithm
  /// \param weights The weights of the edges
  /// \param K The number of shortest paths
  /// \param dij_res The result of dijkstra
  /// \return The (implicit) k shortest paths
  [[nodiscard]] std::vector<implicit_path_info>
  basic_lazy_eppstein(std::function<type_weight(edge_type const &)> &weights,
                      std::size_t                                    K,
                      dijkstra_result_type const                    &dij_res)
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(1)

    auto const &graph = base_shortest::graph;

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return res;
    res.reserve(K);


    auto const sidetrack_distances_res =
      base::sidetrack_distances(weights, dij_res.second); // O(E)
    auto const shortest_path =
      base_shortest::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    H_out_map                   h_out;
    std::map<node_id_type, H_g> h_g;

    auto edges_edges =
      std::map<std::pair<edge_pointer, node_id_type>,
               std::set<std::pair<edge_pointer, node_id_type>>>();


    construct_partial_h_g(
      h_g, h_out, sidetrack_distances_res, successors, 0, edges_edges);

    auto const first_side_track_res =
      base::extrack_first_sidetrack_edge(0, h_g);
    if (!first_side_track_res.first)
      return res;
    auto const &first_side_track = first_side_track_res.second;


    std::set<implicit_path_info> Q;

    implicit_path_info first_path;
    first_path.sidetracks = {
      {first_side_track.edge, first_side_track.edge->first}};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.insert(std::move(first_path));

    auto print_missing_sidetrack_distance = [](edge_type const &e) {
      std::cout << "Error: cannot find proper sidetrack distance for edge ("
                << e.first << ", " << e.second << ")" << std::endl;
    };

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = *Q.begin();
        Q.erase(Q.begin());
        res.push_back(SK);

        auto const  e      = SK.sidetracks.back();
        auto const &e_edge = *e.first;

        auto const ot = sidetrack_distances_res.find(e_edge);

        if (ot == sidetrack_distances_res.cend())
          {
            print_missing_sidetrack_distance(e_edge);
            continue;
          }

        construct_partial_h_g(h_g,
                              h_out,
                              sidetrack_distances_res,
                              successors,
                              e_edge.second,
                              edges_edges);


        auto const f_res =
          base::extrack_first_sidetrack_edge(e_edge.second, h_g);

        if (f_res.first)
          {
            auto const &f = f_res.second;

            auto mod_sk = SK;
            mod_sk.sidetracks.push_back({f.edge, e_edge.second});
            mod_sk.length += f.delta_weight;
            Q.insert(std::move(mod_sk));
          }

        auto const it = edges_edges.find(e);
        if (it != edges_edges.cend())
          {
            SK.sidetracks.pop_back();

            for (auto &f : it->second)
              {
                auto const &f_edge = *f.first;
                auto        ut     = sidetrack_distances_res.find(f_edge);

                if (ut == sidetrack_distances_res.cend())
                  {
                    print_missing_sidetrack_distance(f_edge);
                    continue;
                  }

                auto mod_sk = SK;
                mod_sk.sidetracks.push_back(f);
                mod_sk.length += (ut->second - ot->second);

                if (!SK.sidetracks.empty())
                  {
                    auto n = mod_sk.sidetracks[mod_sk.sidetracks.size() - 2]
                               .first->second;
                    while (n != graph.nodes.size() - 1 &&
                           n != mod_sk.sidetracks.back().first->first)
                      {
                        n = successors[n];
                      }

                    if (n != mod_sk.sidetracks.back().first->first)
                      continue;
                  }

                Q.insert(std::move(mod_sk));
              }
          }
      }


    return res;
  }
};

#endif // NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
