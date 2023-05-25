//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_BUTCHER_H
#define NETWORK_BUTCHER_BUTCHER_H

#include <algorithm>
#include <forward_list>
#include <memory>

#include "Graph_traits.h"

#include "KFinder_Factory.h"

#include "Computer_memory.h"
#include "Parameters.h"
#include "Paths.h"
#include "Utilities.h"
#include "Weight_importers.h"

#include "Constrained_Block_Graph_Builder.h"
#include "Path_Converter.h"

namespace network_butcher
{

  /// Butcher butchers a given graph into slices
  /// \tparam GraphType The type of the graph
  template <typename GraphType>
  class Butcher
  {
  public:
    using network     = GraphType;
    using new_network = block_graph_type;

  private:
    network graph;

  public:
    Butcher() = default;

    explicit Butcher(network &&g)
      : graph(std::move(g)){};

    explicit Butcher(network const &g)
      : graph(g){};

    Butcher(Butcher const &) = delete;

    Butcher
    operator=(Butcher const &) = delete;

    Butcher(Butcher &&d) noexcept = default;

    Butcher &
    operator=(Butcher &&d) noexcept = default;


    /// Basic getter for graph
    /// \return The graph (const reference)
    network const &
    get_graph() const
    {
      return graph;
    }


    /// Basic getter (though simple reference) for graph
    /// \return Reference to the graph
    network &
    get_graph_ref()
    {
      return graph;
    }

    /// This function performs the construction and the butchering of the block graph
    /// \param transmission_weights The transmission weights (i.e. the weight associated to the information transfer
    /// between two different devices
    /// \param params The program parameters
    /// \param extra_constraints A collection of "extra" constraints that can be applied to the block graph after its
    /// construction
    /// \return The optimal partitions
    /// that the K-shortest path algorithm managed to find given the specified constraints
    std::vector<network_butcher::types::Weighted_Real_Path>
    compute_k_shortest_path(
      std::function<weight_type(node_id_type const &, std::size_t, std::size_t)> const &transmission_weights,
      network_butcher::parameters::Parameters const                                    &params,
      std::vector<std::unique_ptr<constraints::Extra_Constraint>> const                &extra_constraints = {}) const;
  };


  template <class GraphType>
  std::vector<network_butcher::types::Weighted_Real_Path>
  Butcher<GraphType>::compute_k_shortest_path(
    const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &transmission_weights,
    const network_butcher::parameters::Parameters                                    &params,
    std::vector<std::unique_ptr<constraints::Extra_Constraint>> const                &extra_constraints) const
  {
    using namespace network_butcher::kfinder;

    // Prepare the builder for the block graph construction
    Constrained_Block_Graph_Builder builder(graph, params, constraints::generate_constraint_function(params, graph));

    // Assemble the weights in the block graph
    builder.construct_weights(transmission_weights);

    // Feed to the builder the collection of extra constraints
    for (auto &constraint : extra_constraints)
      {
        builder.add_constraint(constraint->copy());
      }

    // The actual block graph construction is performed during this step
    auto const new_graph = builder.construct_block_graph();

    // Prepare the K-shortest path algorithm
    auto kFinder = KFinder_Factory<new_network>::Instance().create(params.method,
                                                                   new_graph,
                                                                   new_graph.get_nodes().front().get_id(),
                                                                   new_graph.get_nodes().back().get_id());

    // Find the shortest paths
    auto const res = kFinder->compute(params.K);

    // Convert the result from the block graph to the original graph
    network_butcher::Utilities::Path_Converter converter(new_graph);
    return converter.convert_to_weighted_real_path(res);
  }

} // namespace network_butcher
#endif // NETWORK_BUTCHER_BUTCHER_H