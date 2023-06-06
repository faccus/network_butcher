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


    /// It will generate the callback function using during the Eppstein main loop to construct the required H_gs (and
    /// H_outs) \return The generator function
    auto
    construct_h_g_builder() const;

    /// The basic function for the lazy Eppstein algorithm
    /// \param K The number of shortest paths
    /// \param dij_res The result of dijkstra
    /// \param sidetrack_distances The collection of the sidetrack distances for all the sidetrack edges
    /// \return The (implicit) k shortest paths
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

    h_out.reserve(this->graph.size()); // Reserve space for the h_out collection (O(V)
    h_g.reserve(this->graph.size());   // Reserve space for the h_g collection (O(V)

    auto const &[successors, distances] = dij_res;

    if (distances[Parent_Type::root] == std::numeric_limits<Weight_Type>::max())
      return {};

#if PRINT_DEBUG_STATEMENTS
    Chrono dd_crono;
    dd_crono.start();
#endif

    std::list<Node_Id_Type> to_compute;
    to_compute.push_back(Parent_Type::root);

    while (to_compute.back() != Parent_Type::sink)
      to_compute.push_back(successors[to_compute.back()]);

    auto fun = construct_h_g_builder();

    while (!to_compute.empty())
      {
        fun(h_g, h_out, sidetrack_distances, successors, to_compute.back(), Parent_Type::graph);
        to_compute.pop_back();
      }

#if PRINT_DEBUG_STATEMENTS
    dd_crono.stop();
    std::cout << "Lazy_Eppstein, initial_h_g_build: " << dd_crono.wallTime() / 1000. << " ms" << std::endl;
    dd_crono.start();
#endif

    // Execute the Eppstein algorithm
    return Parent_Type::general_algo_eppstein(K, dij_res, sidetrack_distances, h_g, h_out, fun);
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_h_g_builder() const
  {
    auto const construct_h_out = [](auto       &h_out_collection,
                                    auto const &sidetrack_distances,
                                    auto const &successors,
                                    auto        tail,
                                    auto const &graph) -> auto {
      // If we can find the required H_out, return it
      auto h_out_it = h_out_collection.find(tail);
      if (h_out_it != h_out_collection.cend())
        return h_out_it;

      std::vector<Edge_Info> to_insert;
      auto const            &out_nodes = graph.get_output_nodes(tail);

      to_insert.reserve(sidetrack_distances[tail].size());

      // For every "sidetrack" node in the outer start of node
      for (auto const &exit : out_nodes)
        {
          auto [begin, end] = sidetrack_distances[tail].equal_range(exit);
          for (; begin != end && begin->first == exit; ++begin)
            {
              // Add the sidetrack edges to the H_out
              to_insert.emplace_back(std::make_pair(tail, begin->first), begin->second);
            }
        }

      return h_out_collection.emplace(tail, typename H_out_collection::mapped_type(std::move(to_insert))).first;
    };

    auto const &sink = Parent_Type::sink;

    return [construct_h_out, sink](H_g_collection                        &h_g,
                                   H_out_collection                      &h_out,
                                   Internal_Weight_Collection_Type const &sidetrack_distances,
                                   std::vector<Node_Id_Type> const       &successors,
                                   Node_Id_Type                           node,
                                   auto const                            &graph) {
      auto const internal_builder = [&construct_h_out,
                                     &sink](H_g_collection                        &h_g,
                                            H_out_collection                      &h_out,
                                            Internal_Weight_Collection_Type const &sidetrack_distances,
                                            std::vector<Node_Id_Type> const       &successors,
                                            Node_Id_Type                           node,
                                            auto                                  &func,
                                            auto const                            &graph) {
        // If H_g has been already computed, return it
        auto iterator = h_g.find(node);

        if (iterator != h_g.cend())
          return iterator;

        // Construct and/or retrieve the associated H_out
        auto to_insert_h_out = construct_h_out(h_out, sidetrack_distances, successors, node, graph);

        // If node is not the last node in the graph
        if (node != sink)
          {
            // Construct the H_g of the successor of node in the shortest path
            auto previous_inserted_h_g =
              func(h_g, h_out, sidetrack_distances, successors, successors[node], func, graph);

            // Insert in the successor H_g the current H_out, obtaining the H_g of the current node
            return h_g
              .emplace(node,
                       typename H_g_collection::mapped_type(&(to_insert_h_out->second), previous_inserted_h_g->second))
              .first;
          }
        else
          {
            return h_g.emplace(node, typename H_g_collection::mapped_type(&(to_insert_h_out->second))).first;
          }
      };

      return internal_builder(h_g, h_out, sidetrack_distances, successors, node, internal_builder, graph);
    };
  }
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
