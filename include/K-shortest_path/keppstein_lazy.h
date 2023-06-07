//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
#define NETWORK_BUTCHER_KEPPSTEIN_LAZY_H

#include <list>

#include "basic_keppstein.h"
#include "heap_traits.h"

namespace network_butcher::kfinder
{
  /// This class implements the Lazy Eppstein K-shortest path algorithm
  /// \tparam GraphType The graph type
  template <typename GraphType,
            bool                 Only_Distance                  = false,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<GraphType>>
  class KFinder_Lazy_Eppstein final : public Basic_KEppstein<GraphType, Only_Distance, t_Weighted_Graph_Complete_Type>
  {
  private:
    using Parent_Type = Basic_KEppstein<GraphType, Only_Distance, t_Weighted_Graph_Complete_Type>;

  public:
    using Output_Type = Parent_Type::Output_Type;

  private:
    using Edge_Info   = Parent_Type::Edge_Info;
    using Weight_Type = Parent_Type::Weight_Type;

    using Dijkstra_Result_Type            = Parent_Type::Dijkstra_Result_Type;
    using Internal_Weight_Collection_Type = Parent_Type::Internal_Weight_Collection_Type;

    using H_g_collection   = Parent_Type::H_g_collection;
    using H_out_collection = Parent_Type::H_out_collection;

    /// Simple function that will look for the H_g corresponding to the given node
    /// \param h_g The H_g collections
    /// \param node The node
    /// \return The iterator to the found H_g (or to the end of the collection if not found)
    [[nodiscard]] auto
    find_h_g(H_g_collection &h_g, Node_Id_Type node) const -> H_g_collection::iterator;


    /// It will add to the h_out map the h_out associated to the current node
    /// \param h_out_collection The h_out map
    /// \param sidetrack_distances The sidetrack distances
    /// \param successors The successor collection
    /// \param tail The node associated to the h_out to construct
    /// \return The iterator of the added h_out
    auto
    construct_partial_h_out(H_out_collection                      &h_out_collection,
                            Internal_Weight_Collection_Type const &sidetrack_distances,
                            std::vector<Node_Id_Type> const       &successors,
                            Node_Id_Type                           tail) const -> H_out_collection::const_iterator;


    /// It will add to the h_g map the h_g associated to the current node.
    /// \param h_g The h_g map
    /// \param h_out The h_out map
    /// \param sidetrack_distances The sidetrack distances
    /// \param successors The successor collection
    /// \param node The node associated to the h_out to construct
    /// \return The iterator to the added element
    auto
    construct_partial_h_g(H_g_collection                        &h_g,
                          H_out_collection                      &h_out,
                          Internal_Weight_Collection_Type const &sidetrack_distances,
                          std::vector<Node_Id_Type> const       &successors,
                          Node_Id_Type                           node) const -> H_g_collection::const_iterator;


    /// The basic function for the lazy Eppstein algorithm
    /// \param K The number of shortest paths to compute
    /// \param dij_res The result of the Dijkstra algorithm
    /// \param sidetrack_distances The sidetrack distances of every sidetrack edge
    /// \return The shortest paths (in explicit form)
    [[nodiscard]] auto
    start(std::size_t                            K,
          Dijkstra_Result_Type const            &dij_res,
          Internal_Weight_Collection_Type const &sidetrack_distances) const -> Output_Type override;

  public:
    explicit KFinder_Lazy_Eppstein(GraphType const &g, std::size_t root, std::size_t sink)
      : Parent_Type(g, root, sink){};

    explicit KFinder_Lazy_Eppstein(Weighted_Graph<GraphType> const &g, std::size_t root, std::size_t sink)
      : Parent_Type(g, root, sink){};

    ~KFinder_Lazy_Eppstein() override = default;
  };


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::start(
    std::size_t                            K,
    Dijkstra_Result_Type const            &dij_res,
    Internal_Weight_Collection_Type const &sidetrack_distances) const -> Output_Type
  {
    H_out_collection h_out;
    H_g_collection   h_g;

    auto const &[successors, distances] = dij_res;

    if (distances[Parent_Type::root] == std::numeric_limits<Weight_Type>::max())
      return {};

    for (auto const &node : this->graph)
      {
        if (successors[node.get_id()] != std::numeric_limits<Node_Id_Type>::max())
          {
            h_g.emplace_hint(h_g.end(), node.get_id(), typename H_g_collection::mapped_type());
          }
      }

    std::list<Node_Id_Type> to_compute;
    to_compute.push_back(Parent_Type::root);

    while (to_compute.back() != Parent_Type::sink)
      to_compute.push_back(successors[to_compute.back()]);

    while (!to_compute.empty())
      {
        construct_partial_h_g(h_g, h_out, sidetrack_distances, successors, to_compute.back());
        to_compute.pop_back();
      }

    // Prepare the callback function to be called in the Eppstein algorithm
    auto fun = [this](H_g_collection                                  &h_g_,
                      H_out_collection                                &h_out_,
                      Internal_Weight_Collection_Type const           &sidetrack_distances_,
                      typename Dijkstra_Result_Type::first_type const &successors_,
                      Node_Id_Type                                     node_) {
      return construct_partial_h_g(h_g_, h_out_, sidetrack_distances_, successors_, node_);
    };

    // Execute the Eppstein algorithm
    return Parent_Type::general_algo_eppstein(K, dij_res, sidetrack_distances, h_g, h_out, fun);
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::find_h_g(H_g_collection &h_g,
                                                                                             Node_Id_Type    node) const
    -> H_g_collection::iterator
  {
    return h_g.find(node);
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_partial_h_out(
    H_out_collection                      &h_out_collection,
    Internal_Weight_Collection_Type const &sidetrack_distances,
    std::vector<Node_Id_Type> const       &successors,
    Node_Id_Type                           tail) const -> H_out_collection::const_iterator
  {
    // If we can find the required H_out, return it
    auto h_out_it = h_out_collection.find(tail);
    if (h_out_it != h_out_collection.cend() || successors[tail] == std::numeric_limits<Node_Id_Type>::max())
      return h_out_it;

    auto const &graph = Parent_Type::graph;

    // Prepare the new H_out
    h_out_it = h_out_collection.emplace(tail, typename H_out_collection::mapped_type(tail)).first;

    // For every "sidetrack" node in the outer start of node
    for (auto const &exit : graph.get_output_nodes(tail))
      {
        auto [begin, end] = sidetrack_distances.equal_range(Edge_Type{tail, exit});
        for (; begin != end; ++begin)
          {
            // Add the sidetrack edges to the H_out
            h_out_it->second.push(Edge_Info{begin->first, begin->second});
          }
      }

    return h_out_it;
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_partial_h_g(
    H_g_collection                        &h_g,
    H_out_collection                      &h_out,
    Internal_Weight_Collection_Type const &sidetrack_distances,
    std::vector<Node_Id_Type> const       &successors,
    Node_Id_Type                           node) const -> H_g_collection::const_iterator
  {
    // If H_g has been already computed, return it
    auto iterator = find_h_g(h_g, node);

    if (iterator == h_g.cend() || iterator->second.is_id_set())
      return iterator;

    iterator->second.id = node;

    // If node is the last node in the graph
    if (node != Parent_Type::sink)
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
