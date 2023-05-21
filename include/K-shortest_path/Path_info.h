//
// Created by faccus on 12/12/21.
//

#ifndef NETWORK_BUTCHER_PATH_INFO_H
#define NETWORK_BUTCHER_PATH_INFO_H

#include "Basic_traits.h"
#include "crtp_grater.h"

namespace network_butcher::kfinder
{
  /// Simple struct to represent an explicit path
  struct path_info : crtp_greater<path_info>
  {
    weight_type               length;
    std::vector<node_id_type> path;

    constexpr bool
    operator<(const path_info &rhs) const
    {
      return length < rhs.length || (length == rhs.length && path < rhs.path);
    }
  };
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_PATH_INFO_H
