//
// Created by faccus on 5/24/22.
//

#ifndef NETWORK_BUTCHER_PATHS_H
#define NETWORK_BUTCHER_PATHS_H

#include <vector>

namespace network_butcher
{
  namespace types
  {
    using Slice_Type = std::set<node_id_type>;

    using Real_Partition = std::pair<std::size_t, Slice_Type>;
    using Real_Path      = std::vector<Real_Partition>;

    using Weighted_Real_Path  = std::pair<weight_type, Real_Path>;
    using Weighted_Real_Paths = std::vector<Weighted_Real_Path>;
  } // namespace types
} // namespace network_butcher

#endif // NETWORK_BUTCHER_PATHS_H
