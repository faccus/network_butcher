//
// Created by faccus on 29/05/23.
//

#ifndef NETWORK_BUTCHER_STARTING_TRAITS_H
#define NETWORK_BUTCHER_STARTING_TRAITS_H

#include <set>

namespace network_butcher
{
  using type_info_id_type = int;
  using shape_type        = unsigned long;

  using memory_type = unsigned long long;
  using weight_type = long double;

  using bandwidth_type    = double;
  using access_delay_type = double;

  using node_id_type = long unsigned int;
} // namespace network_butcher

namespace network_butcher
{
  using node_id_collection_type = std::set<node_id_type>;
  using edge_type               = std::pair<node_id_type, node_id_type>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_STARTING_TRAITS_H
