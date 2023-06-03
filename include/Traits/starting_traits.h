//
// Created by faccus on 03/06/23.
//

#ifndef NETWORK_BUTCHER_STARTING_TRAITS_H
#define NETWORK_BUTCHER_STARTING_TRAITS_H

namespace network_butcher
{
  using Type_Info_Id_Type       = int;
  using Onnx_Element_Shape_Type = unsigned long;

  using Memory_Type = unsigned long long;
  using Time_Type   = long double;

  using Bandwidth_Value_Type    = double;
  using Access_Delay_Value_Type = double;

  using Node_Id_Type = long unsigned int;
} // namespace network_butcher

#endif // NETWORK_BUTCHER_STARTING_TRAITS_H
