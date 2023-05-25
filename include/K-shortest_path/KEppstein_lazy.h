//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
#define NETWORK_BUTCHER_KEPPSTEIN_LAZY_H

#include "Heap_traits.h"
#include "basic_KEppstein.h"

namespace network_butcher::kfinder
{
  /// This class implements the Lazy Eppstein K-shortest path algorithm
  /// \tparam Graph_type The graph type
  template <typename Graph_type,
            bool                 Only_Distance                  = false,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<Graph_type>>
  class KFinder_Lazy_Eppstein final : public basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>
  {
  private:
    using base = basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>;

  public:
    using Output_Type = typename base::Output_Type;

  private:
    using Weight_Type = base::Weight_Type;

    using edge_info = base::edge_info;

    using internal_weight_collection = base::internal_weight_collection;
    using dijkstra_result_type       = base::dijkstra_result_type;

    using H_g_collection   = base::H_g_collection;
    using H_out_collection = base::H_out_collection;

    /// Simple function that will look for the H_g corresponding to the given node
    /// \param h_g The H_g collections
    /// \param node The node
    /// \return A pair: a boolean that is true if the relevant H_g is found and the relevant iterator to the
    /// relevant H_g
    H_g_collection::iterator
    find_h_g(H_g_collection &h_g, node_id_type node) const;


    /// It will add to the h_out map the h_out associates to the current node
    /// \param h_out_collection The h_out map
    /// \param sidetrack_distances The sidetrack distances
    /// \param successors The successor collection
    /// \param tail The node associated to the h_out to construct
    /// \return The iterator of the added h_out
    H_out_collection::const_iterator
    construct_partial_h_out(H_out_collection                 &h_out_collection,
                            internal_weight_collection const &sidetrack_distances,
                            std::vector<node_id_type> const  &successors,
                            node_id_type                      tail) const;

    /// It will add to the h_g map the h_g associated to the current node. It will also update the edge_edges map
    /// (that associated every edge to its children)
    /// \param h_g The h_g map
    /// \param h_out The h_out map
    /// \param sidetrack_distances The sidetrack distances
    /// \param successors The successor collection
    /// \param node The node associated to the h_out to construct
    /// \return The iterator to the added element
    H_g_collection::const_iterator
    construct_partial_h_g(H_g_collection                   &h_g,
                          H_out_collection                 &h_out,
                          internal_weight_collection const &sidetrack_distances,
                          std::vector<node_id_type> const  &successors,
                          node_id_type                      node) const;

    /// The basic function for the lazy Eppstein algorithm
    /// \param K The number of shortest paths
    /// \param dij_res The result of dijkstra
    /// \return The (implicit) k shortest paths
    [[nodiscard]] Output_Type
    start(std::size_t K, dijkstra_result_type const &dij_res) const override;
    ;

  public:
    explicit KFinder_Lazy_Eppstein(Graph_type const &g, std::size_t root, std::size_t sink)
      : base(g, root, sink){};

    explicit KFinder_Lazy_Eppstein(Weighted_Graph<Graph_type> const &g, std::size_t root, std::size_t sink)
      : base(g, root, sink){};

    ~KFinder_Lazy_Eppstein() override = default;
  };


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::Output_Type
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::start(
    std::size_t                                        K,
    const KFinder_Lazy_Eppstein::dijkstra_result_type &dij_res) const
  {
    auto const sidetrack_distances_res = base::sidetrack_distances(dij_res); // O(E)

    H_out_collection h_out;
    H_g_collection   h_g;

    auto const &[successors, distances] = dij_res;

    if (distances[base::root] == std::numeric_limits<Weight_Type>::max())
      return {};

    for (auto const &node : this->graph)
      {
        if (successors[node.get_id()] != std::numeric_limits<node_id_type>::max())
          {
            h_g.emplace_hint(h_g.end(), node.get_id(), typename H_g_collection::mapped_type());
          }
      }

    std::list<node_id_type> to_compute;
    to_compute.push_back(base::root);

    while (to_compute.back() != base::sink)
      to_compute.push_back(successors[to_compute.back()]);

    while (!to_compute.empty())
      {
        construct_partial_h_g(h_g, h_out, sidetrack_distances_res, successors, to_compute.back());
        to_compute.pop_back();
      }

    // Prepare the callback function to be called in the Eppstein algorithm
    auto fun = [this](H_g_collection                                  &h_g_,
                      H_out_collection                                &h_out_,
                      internal_weight_collection const                &sidetrack_distances_,
                      typename dijkstra_result_type::first_type const &successors_,
                      node_id_type                                     node_) {
      return construct_partial_h_g(h_g_, h_out_, sidetrack_distances_, successors_, node_);
    };

    // Execute the Eppstein algorithm
    auto epp_res = base::general_algo_eppstein(K, dij_res, sidetrack_distances_res, h_g, h_out, fun);

    if constexpr (Only_Distance)
      {
        return epp_res;
      }
    else
      {
        return base::helper_eppstein(dij_res, epp_res);
      }
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::H_g_collection::iterator
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::find_h_g(H_g_collection &h_g,
                                                                                             node_id_type    node) const
  {
    return h_g.find(node);
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::H_out_collection::const_iterator
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_partial_h_out(
    H_out_collection                 &h_out_collection,
    const internal_weight_collection &sidetrack_distances,
    const std::vector<node_id_type>  &successors,
    node_id_type                      tail) const
  {
    // If we can find the required H_out, return it
    auto h_out_it = h_out_collection.find(tail);
    if (h_out_it != h_out_collection.cend() || successors[tail] == std::numeric_limits<node_id_type>::max())
      return h_out_it;

    auto const &graph = base::graph;

    // Prepare the new H_out
    h_out_it = h_out_collection.emplace(tail, typename H_out_collection::mapped_type(tail)).first;

    // For every "sidetrack" node in the outer start of node
    for (auto const &exit : graph.get_output_nodes(tail))
      {
        auto [begin, end] = sidetrack_distances.equal_range(edge_type{tail, exit});
        for (; begin != end; ++begin)
          {
            // Add the sidetrack edges to the H_out
            h_out_it->second.push(edge_info{begin->first, begin->second});
          }
      }

    return h_out_it;
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::H_g_collection::const_iterator
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_partial_h_g(
    H_g_collection                   &h_g,
    H_out_collection                 &h_out,
    const internal_weight_collection &sidetrack_distances,
    const std::vector<node_id_type>  &successors,
    node_id_type                      node) const
  {
    // If H_g has been already computed, return it
    auto iterator = find_h_g(h_g, node);

    if (iterator == h_g.cend() || iterator->second.is_id_set())
      return iterator;

    iterator->second.id = node;

    // If node is the last node in the graph
    if (node != base::sink)
      {
        // Construct the H_g of the successor of node in the shortest path
        auto previous_inserted_h_g =
          construct_partial_h_g(h_g, h_out, sidetrack_distances, successors, successors[node]);

        // Prepare a new H_g
        if (previous_inserted_h_g != h_g.end() && !previous_inserted_h_g->second.empty())
          iterator->second.overwrite_children(previous_inserted_h_g->second);
      }

    // Construct and/or retrieve the associated H_out
    auto to_insert_h_out = construct_partial_h_out(h_out, sidetrack_distances, successors, node);

    if (to_insert_h_out != h_out.cend() && !to_insert_h_out->second.empty())
      {
        iterator->second.push(to_insert_h_out);
      }

    return iterator;
  }
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
