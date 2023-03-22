//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_TYPE_INFO_TRAITS_H
#define NETWORK_BUTCHER_TYPE_INFO_TRAITS_H

#include <string>

namespace network_butcher
{
  using type_info_id_type = int;
  using shape_type        = std::size_t;

  using memory_type    = std::size_t;
  using bandwidth_type = double;
} // namespace network_butcher

#endif // NETWORK_BUTCHER_TYPE_INFO_TRAITS_H
