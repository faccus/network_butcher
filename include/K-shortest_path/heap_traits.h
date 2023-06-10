#ifndef NETWORK_BUTCHER_HEAP_TRAITS_H
#define NETWORK_BUTCHER_HEAP_TRAITS_H

#include "heap_eppstein.h"
#include "path_info.h"
#include "traits.h"
#include "weighted_graph.h"


namespace network_butcher::kfinder
{
  /// Simple alias for a map storing H_out_Type, indexed by the node id. Each H_out_Type will store a
  /// Templated_Edge_Info<Weight_Type>
  template <typename Weight_Type = Time_Type>
  using Templated_H_out_Collection =
    std::unordered_map<Node_Id_Type, H_out_Type<Templated_Edge_Info<Weight_Type>, std::less<>>>;

  /// Simple alias for a map storing H_g_Type, indexed by the node id.
  template <typename Weight_Type = Time_Type>
  using Templated_H_g_Collection =
    std::unordered_map<Node_Id_Type, H_g_Type<Templated_Edge_Info<Weight_Type>, std::less<>>>;
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_TRAITS_H
