#ifndef NETWORK_BUTCHER_STARTING_TRAITS_H
#define NETWORK_BUTCHER_STARTING_TRAITS_H

namespace network_butcher
{
  /// Type used to store the length in byte of a tensor
  using Memory_Type = unsigned long long;

  /// Basic weight type (seconds, s)
  using Time_Type   = long double;

  /// Bandwidth speed value type (megabit per second, MBit/s)
  using Bandwidth_Value_Type    = double;

  /// Access delay type (seconds, s)
  using Access_Delay_Value_Type = double;

  /// The id of a node in a graph. Used also to represent device ids
  using Node_Id_Type = long unsigned int;
} // namespace network_butcher

#endif // NETWORK_BUTCHER_STARTING_TRAITS_H
