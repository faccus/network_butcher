#ifndef NETWORK_BUTCHER_PATHS_H
#define NETWORK_BUTCHER_PATHS_H

#include <set>
#include <vector>

namespace network_butcher::types
{
  using Slice_Type = std::set<Node_Id_Type>;

  using Real_Partition = std::pair<std::size_t, Slice_Type>;
  using Real_Path      = std::vector<Real_Partition>;

  using Weighted_Real_Path  = std::pair<Time_Type, Real_Path>;

  bool
  path_comparison(Weighted_Real_Path const &rhs, Weighted_Real_Path const &lhs);
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_PATHS_H
