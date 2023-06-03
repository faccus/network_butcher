//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_BASIC_TRAITS_H
#define NETWORK_BUTCHER_BASIC_TRAITS_H

#include <set>
#include <map>
#include <memory>

#include "starting_traits.h"

#include "onnx.pb.h"
#include "type_info.h"


#if PARALLEL_TBB
#include <execution>
#elif PARALLEL_OPENMP
#include <omp.h>
#endif


namespace network_butcher
{
  using Node_Id_Collection_Type = std::set<Node_Id_Type>;
  using Edge_Type               = std::pair<Node_Id_Type, Node_Id_Type>;
  using Type_Info_Pointer = std::shared_ptr<network_butcher::types::Type_Info const>;

  template <class T = Type_Info_Pointer>
  using Io_Collection_Type = std::map<std::string, T>;

  template <typename T>
  using Repeatable_field = google::protobuf::RepeatedPtrField<T>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_BASIC_TRAITS_H
