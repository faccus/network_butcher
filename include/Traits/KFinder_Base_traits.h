//
// Created by faccus on 21/05/23.
//

#ifndef NETWORK_BUTCHER_KFINDER_BASE_TRAITS_H
#define NETWORK_BUTCHER_KFINDER_BASE_TRAITS_H

#include <type_traits>

#include "Weighted_Graph.h"

namespace network_butcher::kfinder
{
  template <typename T>
  concept Valid_Weighted_Graph = std::is_base_of_v<base_Weighted_Graph, T>;
}

#endif // NETWORK_BUTCHER_KFINDER_BASE_TRAITS_H
