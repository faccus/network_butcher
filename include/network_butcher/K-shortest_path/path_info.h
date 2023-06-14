#ifndef NETWORK_BUTCHER_PATH_INFO_H
#define NETWORK_BUTCHER_PATH_INFO_H

#include <network_butcher/K-shortest_path/crtp_greater.h>
#include <network_butcher/Traits/traits.h>

namespace network_butcher::kfinder
{
  /// Simple struct to represent an explicit path
  template <typename Weight_Type = Time_Type>
  struct Templated_Path_Info : Crtp_Greater<Templated_Path_Info<Weight_Type>>
  {
    /// Path length
    Weight_Type               length;

    /// Collection of nodes visited during the path
    std::vector<Node_Id_Type> path;

    bool
    operator<(const Templated_Path_Info<Weight_Type> &rhs) const
    {
      return length < rhs.length || (length == rhs.length && path < rhs.path);
    }
  };
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_PATH_INFO_H
