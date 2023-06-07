#ifndef NETWORK_BUTCHER_KFINDER_BASE_TRAITS_H
#define NETWORK_BUTCHER_KFINDER_BASE_TRAITS_H

#include <type_traits>

#include "weighted_graph.h"

namespace network_butcher::kfinder
{
  template <typename T>
  concept Valid_Weighted_Graph = std::is_base_of_v<Base_Weighted_Graph, T>;
}

#endif // NETWORK_BUTCHER_KFINDER_BASE_TRAITS_H
