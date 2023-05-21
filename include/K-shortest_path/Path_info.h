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
  template <typename Weight_Type = weight_type>
  struct t_path_info : crtp_greater<t_path_info<Weight_Type>>
  {
    Weight_Type               length;
    std::vector<node_id_type> path;

    constexpr bool
    operator<(const t_path_info &rhs) const
    {
      return length < rhs.length || (length == rhs.length && path < rhs.path);
    }
  };
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_PATH_INFO_H
