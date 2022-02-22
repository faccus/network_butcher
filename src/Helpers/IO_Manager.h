//
// Created by faccus on 20/02/22.
//

#ifndef NETWORK_BUTCHER_IO_MANAGER_H
#define NETWORK_BUTCHER_IO_MANAGER_H

#include "Traits/Graph_traits.h"

class IO_Manager
{
private:
  using Map_IO = std::unordered_map<std::string, type_info_pointer>;

  static void
  onnx_io_read(
    Map_IO                                                         &input_map,
    google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> const &collection,
    std::unordered_set<std::string> const &initialized);

  static void
  onnx_process_node(
    google::protobuf::RepeatedPtrField<std::basic_string<char>> const &io_names,
    io_collection_type<type_info_pointer> &io_collection,
    io_collection_type<type_info_pointer> &parameters_collection,
    Map_IO const                          &value_infos);

public:
  static std::pair<graph_type, onnx::ModelProto>
  import_from_onnx(std::string const &path);

  static void
  export_to_onnx(std::pair<graph_type, onnx::ModelProto> const &);
};


#endif // NETWORK_BUTCHER_IO_MANAGER_H