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
  std::pair<bool, H_g_collection::iterator>
  find_h_g_in_map(H_g_collection &h_g, node_id_type node)
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
  H_out_collection::iterator
  construct_partial_h_out(H_out_collection                &h_out,
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
  H_g_collection::iterator
  construct_partial_h_g(H_g_collection                  &h_g,
                        H_out_collection                &h_out,
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
    auto const sidetrack_distances_res =
      base::sidetrack_distances(dij_res.second); // O(E)


    H_out_collection h_out;
    H_g_collection   h_g;

    auto const &successors = dij_res.first;

    construct_partial_h_g(h_g, h_out, sidetrack_distances_res, successors, 0);

    typename base::callback_function_helper_eppstein fun =
      [this](H_g_collection                  &h_g_,
             H_out_collection                &h_out_,
             weights_collection_type const   &sidetrack_distances_,
             std::vector<node_id_type> const &successors_,
             node_id_type                     node_) {
        construct_partial_h_g(
          h_g_, h_out_, sidetrack_distances_, successors_, node_);
      };

    return base::helper_eppstein_support(
      K, dij_res, sidetrack_distances_res, h_g, h_out, true, fun);
  }
};

#endif // NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
