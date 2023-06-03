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
  using Templated_H_out_Collection = std::unordered_map<Node_Id_Type, H_out<Templated_Edge_Info<Weight_Type>, std::greater<>>>;

  template <typename Weight_Type = Time_Type>
  using H_out_Template_Iterator = Templated_H_out_Collection<Weight_Type>::const_iterator;

  template <typename Weight_Type = Time_Type>
  struct Pointer_Greater
  {
    std::less<> comp{};

    bool
    operator()(Templated_H_out_Collection<Weight_Type>::const_iterator const &lhs,
               Templated_H_out_Collection<Weight_Type>::const_iterator const &rhs) const
    {
      return comp(rhs->second, lhs->second);
    }
  };

  template <typename Weight_Type = Time_Type>
  using Templated_H_g_Collection =
    std::unordered_map<Node_Id_Type, Heap<H_out_Template_Iterator<Weight_Type>, Pointer_Greater<Weight_Type>>>;
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_TRAITS_H
