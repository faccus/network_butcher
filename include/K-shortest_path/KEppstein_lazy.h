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
  template <class Graph_type, bool Only_Distance = false>
  class KFinder_Lazy_Eppstein final : public basic_KEppstein<Graph_type, Only_Distance>
  {
  private:
    using base                       = basic_KEppstein<Graph_type, Only_Distance>;
    using internal_weight_collection = basic_KEppstein<Graph_type, Only_Distance>::internal_weight_collection;
    using dijkstra_result_type       = basic_KEppstein<Graph_type, Only_Distance>::dijkstra_result_type;
    using implicit_path_info         = basic_KEppstein<Graph_type, Only_Distance>::implicit_path_info;

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
    [[nodiscard]] std::conditional_t<Only_Distance, std::vector<weight_type>, std::vector<path_info>>
    start(std::size_t K, dijkstra_result_type const &dij_res) const override;

  public:
    explicit KFinder_Lazy_Eppstein(Graph_type const &g, std::size_t root, std::size_t sink)
      : base(g, root, sink){};

    explicit KFinder_Lazy_Eppstein(Weighted_Graph<Graph_type> const &g, std::size_t root, std::size_t sink)
      : base(g, root, sink){};

    ~KFinder_Lazy_Eppstein() override = default;
  };


  template <class Graph_type, bool Only_Distance>
  H_g_collection::iterator
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance>::find_h_g(H_g_collection &h_g, node_id_type node) const
  {
    return h_g.find(node);
  }


  template <class Graph_type, bool Only_Distance>
  H_out_collection::const_iterator
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance>::construct_partial_h_out(
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
    h_out_it = h_out_collection.emplace(tail, H_out_collection::mapped_type(tail)).first;

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


  template <class Graph_type, bool Only_Distance>
  H_g_collection::const_iterator
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance>::construct_partial_h_g(
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

    auto const &sink    = KFinder<Graph_type, Only_Distance>::sink;
    iterator->second.id = node;

    // If node is the last node in the graph
    if (node != sink)
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


  template <class Graph_type, bool Only_Distance>
  std::conditional_t<Only_Distance, std::vector<weight_type>, std::vector<path_info>>
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance>::start(std::size_t K, const dijkstra_result_type &dij_res) const
  {
    auto const &root = KFinder<Graph_type, Only_Distance>::root;

    auto const sidetrack_distances_res = base::sidetrack_distances(dij_res); // O(E)

    H_out_collection h_out;
    H_g_collection   h_g;

    auto const &successors = dij_res.first;

#if LOCAL
    Chrono crono;
    crono.start();
#endif

    for (auto const &node : this->graph)
      {
        if (successors[node.get_id()] != std::numeric_limits<node_id_type>::max())
          {
            h_g.emplace_hint(h_g.end(), node.get_id(), H_g_collection::mapped_type());
          }
      }


    // Firstly, compute the H_g for the source node and check if it was successfully constructed
    if (construct_partial_h_g(h_g, h_out, sidetrack_distances_res, successors, root) == h_g.cend())
      {
        // If not... there is no shortest path...
        return {};
      }

#if LOCAL
    crono.stop();
    std::cout << "Initial H_g construction time: " << crono.wallTime() / 1000. / 1000. << " s" << std::endl;
#endif


    // Prepare the callback function to be called in the Eppstein algorithm
    auto fun = [this](H_g_collection                   &h_g_,
                      H_out_collection                 &h_out_,
                      internal_weight_collection const &sidetrack_distances_,
                      std::vector<node_id_type> const  &successors_,
                      node_id_type                      node_) {
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
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
