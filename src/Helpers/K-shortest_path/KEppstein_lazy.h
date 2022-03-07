//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
#define NETWORK_BUTCHER_KEPPSTEIN_LAZY_H

#include "KFinder.h"
#include "../Traits/Heap_traits.h"

template <class Graph_type>
class KFinder_Lazy_Eppstein : public KFinder<Graph_type>
{
public:
  using base          = KFinder<Graph_type>;
  using base_shortest = Shortest_path_finder<Graph_type>;


  /// Applies the lazy Eppstein algorithm to find the k-shortest paths on the
  /// given graph (from the first node to the last one)
  /// \param weights The weights associated to the different edges
  /// \param K The number of shortest paths to find
  /// \return The shortest paths
  [[nodiscard]] std::vector<path_info>
  lazy_eppstein(std::size_t K)
  {
    auto const &graph = base_shortest::graph;

    if (graph.get_nodes().empty() || K == 0)
      return {};

    auto const dij_res =
      base_shortest::shortest_path_tree(); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base_shortest::shortest_path_finder(dij_res, 0)};


    auto const epp_res = basic_lazy_eppstein(K, dij_res);

    return base::helper_eppstein(dij_res, epp_res);
  }

  explicit KFinder_Lazy_Eppstein(Graph_type const &g)
    : base(g){};

  virtual ~KFinder_Lazy_Eppstein() = default;

private:
  std::pair<bool, H_g_map::iterator>
  find_h_g_in_map(H_g_map &h_g, node_id_type node)
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
                          weights_collection_type const   &sidetrack_distances,
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

    for (auto const &exit : graph.get_dependencies()[node].second)
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

          children.insert(children.cend(), std::move(tmp)); // O(log(N))
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
  H_g_map::iterator
  construct_partial_h_g(H_g_map                         &h_g,
                        H_out_map                       &h_out,
                        weights_collection_type const   &sidetrack_distances,
                        std::vector<node_id_type> const &successors,
                        node_id_type                     node)
  {
    auto pair_iterator = find_h_g_in_map(h_g, node);
    if (pair_iterator.first)
      return pair_iterator.second;

    auto const &graph = base_shortest::graph;


    if (node == graph.get_nodes().size() - 1)
      {
        auto inserted_h_g = h_g.emplace(node, H_g()).first;

        auto to_insert_h_out =
          construct_partial_h_out(h_out, sidetrack_distances, successors, node);

        if (!to_insert_h_out->second->heap.children.empty())
          inserted_h_g->second.children.insert(to_insert_h_out->second);

        return inserted_h_g;
      }

    auto const successor = successors[node];

    auto previous_inserted_h_g = construct_partial_h_g(
      h_g, h_out, sidetrack_distances, successors, successor);

    auto inserted_h_g = h_g.emplace(node, previous_inserted_h_g->second);
    inserted_h_g.first->second.id = node;

    auto current_node_h_out =
      construct_partial_h_out(h_out, sidetrack_distances, successors, node);


    if (!current_node_h_out->second->heap.children.empty())
      {
        inserted_h_g.first->second.children.insert(current_node_h_out->second);
      }

    return inserted_h_g.first;
  }

  /// The basic function for the lazy Eppstein algorithm
  /// \param weights The weights of the edges
  /// \param K The number of shortest paths
  /// \param dij_res The result of dijkstra
  /// \return The (implicit) k shortest paths
  [[nodiscard]] std::vector<implicit_path_info>
  basic_lazy_eppstein(std::size_t K, dijkstra_result_type const &dij_res)
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()}); // O(1)

    auto const &graph = base_shortest::graph;
    auto const &nodes = graph.get_nodes();

    if (nodes.empty())
      return {};
    if (K == 1)
      return res;

    res.reserve(K);

    auto const sidetrack_distances_res =
      base::sidetrack_distances(dij_res.second); // O(E)
    auto const shortest_path =
      base_shortest::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    H_out_map                   h_out;
    H_g_map h_g;

    edge_edges_type h_out_edge_edges;
    edge_edges_type h_g_edge_edges;

    construct_partial_h_g(
      h_g, h_out, sidetrack_distances_res, successors, 0);

    auto const first_side_track_res =
      base::extrack_first_sidetrack_edge(0, h_g);
    if (!first_side_track_res.first)
      return res;
    auto const &first_side_track = first_side_track_res.second;

    std::set<implicit_path_info> Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
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
        auto const &e_edge = *e;

        auto const ot = sidetrack_distances_res.find(e_edge);

        if (ot == sidetrack_distances_res.cend())
          {
            print_missing_sidetrack_distance(e_edge);
            continue;
          }

        construct_partial_h_g(
          h_g, h_out, sidetrack_distances_res, successors, e_edge.second);


        auto const f_res =
          base::extrack_first_sidetrack_edge(e_edge.second, h_g);

        if (f_res.first)
          {
            auto const &f = f_res.second;

            auto mod_sk = SK;
            mod_sk.sidetracks.push_back(f.edge);
            mod_sk.length += f.delta_weight;
            Q.insert(std::move(mod_sk));
          }

        node_id_type h_g_search;
        if (SK.sidetracks.size() == 1)
          h_g_search = 0;
        else
          {
            auto const tmp_it = ++SK.sidetracks.crbegin();
            h_g_search        = (*tmp_it)->second;
          }

        auto const alternatives = base::get_alternatives(
          h_g.find(h_g_search)->second, h_g_edge_edges, h_out_edge_edges, e);

        if (!alternatives.empty())
          {
            SK.sidetracks.pop_back();

            for (auto const &f : alternatives)
              {
                auto const &f_edge = *f;
                auto        ut     = sidetrack_distances_res.find(f_edge);

                if (ut == sidetrack_distances_res.cend())
                  {
                    print_missing_sidetrack_distance(f_edge);
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

#endif // NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
