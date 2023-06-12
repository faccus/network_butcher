#ifndef NETWORK_BUTCHER_TRAITS_H
#define NETWORK_BUTCHER_TRAITS_H

#include <set>
#include <map>
#include <memory>

#include <network_butcher/Traits/starting_traits.h>
#include <network_butcher/Types/type_info.h>

#include <network_butcher/onnx_proto/onnx.pb.h>


#if NETWORK_BUTCHER_PARALLEL_TBB
#include <execution>
#elif NETWORK_BUTCHER_PARALLEL_OPENMP
#include <omp.h>
#endif


namespace network_butcher
{
  /// Collection of node ids
  using Node_Id_Collection_Type = std::set<Node_Id_Type>;

  /// Edge in a graph. Simple pair of node ids
  using Edge_Type               = std::pair<Node_Id_Type, Node_Id_Type>;

  /// Shared pointer to a Type_info
  using Type_Info_Pointer = std::shared_ptr<network_butcher::types::Type_Info const>;

  /// Simple map that stores objects of type T indexed by a string
  template <class T = Type_Info_Pointer>
  using Io_Collection_Type = std::map<std::string, T>;

  /// Simple alias for google::protobuf::RepeatedPtrField<T>
  template <typename T>
  using RepeatablePtr_field = google::protobuf::RepeatedPtrField<T>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_TRAITS_H
