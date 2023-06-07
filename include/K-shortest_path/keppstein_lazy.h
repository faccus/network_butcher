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

    using Callback_Function = Parent_Type::Callback_Function;

    /// It will generate the callback function used during the Eppstein main loop to construct the required H_gs (and
    /// H_outs)
    /// \return The generator function
    auto
    construct_h_g_builder() const -> Callback_Function;


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
    auto const &[successors, distances] = dij_res;

    if (distances[Parent_Type::root] == std::numeric_limits<Weight_Type>::max())
      return {};

    H_out_collection h_out;
    h_out.reserve(this->graph.size());

    H_g_collection h_g;
    h_g.reserve(this->graph.size());

    std::list<Node_Id_Type> to_compute;
    to_compute.push_back(Parent_Type::root);

    while (to_compute.back() != Parent_Type::sink)
      to_compute.push_back(successors[to_compute.back()]);

    // Prepare the callback function to be called in the Eppstein algorithm
    auto h_g_builder = construct_h_g_builder();

    while (!to_compute.empty())
      {
        h_g_builder(h_g, h_out, sidetrack_distances, successors, to_compute.back(), this->graph);
        to_compute.pop_back();
      }

    // Execute the Eppstein algorithm
    return Parent_Type::general_algo_eppstein(K, dij_res, sidetrack_distances, h_g, h_out, h_g_builder);
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Lazy_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_h_g_builder() const
    -> Callback_Function
  {
    auto const construct_h_out = [](H_out_collection                      &h_out_collection,
                                    Internal_Weight_Collection_Type const &sidetrack_distances,
                                    std::vector<Node_Id_Type> const       &successors,
                                    Node_Id_Type                           tail,
                                    auto const                            &graph) {
      // If we can find the required H_out, return it
      auto h_out_it = h_out_collection.find(tail);
      if (h_out_it != h_out_collection.cend() || successors[tail] == std::numeric_limits<Node_Id_Type>::max())
        return h_out_it;

      // Prepare the new H_out
      h_out_it = h_out_collection.emplace(tail, typename H_out_collection::mapped_type()).first;

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
    };

    auto const &sink = Parent_Type::sink;

    // Extra lambda required to use recursion
    auto const internal_builder = [construct_h_out, sink](H_g_collection                        &h_g,
                                                          H_out_collection                      &h_out,
                                                          Internal_Weight_Collection_Type const &sidetrack_distances,
                                                          std::vector<Node_Id_Type> const       &successors,
                                                          Node_Id_Type                           node,
                                                          auto const                            &graph,
                                                          auto const                            &func) {
      // If H_g has been already computed, return it
      auto iterator = h_g.find(node);

      if (iterator != h_g.cend())
        return iterator;

      // Construct and/or retrieve the associated H_out
      auto to_insert_h_out = construct_h_out(h_out, sidetrack_distances, successors, node, graph);

      // Construct the H_g
      auto build_iterator = h_g.emplace(node, typename H_g_collection::mapped_type()).first;

      // If node is the last node in the graph
      if (node != sink)
        {
          // Construct the H_g of the successor of node in the shortest path
          auto previous_inserted_h_g = func(h_g, h_out, sidetrack_distances, successors, successors[node], graph, func);

          // Prepare a new H_g
          if (previous_inserted_h_g != h_g.end() && !previous_inserted_h_g->second.empty())
            build_iterator->second.overwrite_children(previous_inserted_h_g->second);
        }

      if (to_insert_h_out != h_out.cend() && !to_insert_h_out->second.empty())
        {
          build_iterator->second.push(to_insert_h_out);
        }

      return build_iterator;
    };

    return [internal_builder](H_g_collection                        &h_g,
                              H_out_collection                      &h_out,
                              Internal_Weight_Collection_Type const &sidetrack_distances,
                              std::vector<Node_Id_Type> const       &successors,
                              Node_Id_Type                           node,
                              t_Weighted_Graph_Complete_Type const  &graph) {
      return internal_builder(h_g, h_out, sidetrack_distances, successors, node, graph, internal_builder);
    };
  }
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_KEPPSTEIN_LAZY_H
