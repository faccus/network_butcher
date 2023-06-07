//
// Created by root on 07/03/22.
//

#ifndef NETWORK_BUTCHER_HEAP_TRAITS_H
#define NETWORK_BUTCHER_HEAP_TRAITS_H

#include "basic_traits.h"
#include "heap_eppstein.h"
#include "path_info.h"
#include "weighted_graph.h"


namespace network_butcher::kfinder
{
  template <typename Weight_Type = Time_Type>
  using Templated_H_out_Collection =
    std::unordered_map<Node_Id_Type, H_out_Type<Templated_Edge_Info<Weight_Type>, std::less<>>>;

  template <typename Weight_Type = Time_Type>
  using Templated_H_g_Collection =
    std::unordered_map<Node_Id_Type, H_g_Type<Templated_Edge_Info<Weight_Type>, std::less<>>>;
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_TRAITS_H
