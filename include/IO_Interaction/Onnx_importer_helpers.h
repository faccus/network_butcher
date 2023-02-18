//
// Created by faccus on 8/7/22.
//

#ifndef NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H
#define NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H

#include "Butcher.h"
#include "Graph_traits.h"
#include "Parameters.h"

/// This namespace contains all the methods required to import a .onnx model to a Graph object
namespace network_butcher_io::Onnx_importer_helpers
{
  using Map_IO = std::unordered_map<std::string, type_info_pointer>;


  /// Inserts into the input_map the valid elements (onnx::ValueInfoProto) contained in collection and
  /// whether they are initialized or not
  /// \param input_map The input_map
  /// \param collection The collection of IO elements
  /// \param initialized The collection of names of the initialized IO elements
  void
  read_ios(Map_IO                                                         &input_map,
           google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> const &collection,
           std::set<std::string> const                                    &initialized);

  /// Inserts into the input_map the valid elements (onnx::TensorProto) contained in collection and
  /// whether they are initialized or not
  /// \param input_map The input_map
  /// \param collection The collection of IO elements
  /// \param initialized The collection of names of the initialized IO elements
  void
  read_ios(Map_IO                                                      &input_map,
           google::protobuf::RepeatedPtrField<onnx::TensorProto> const &collection,
           std::set<std::string> const                                 &initialized);

  /// It will look for unused IOs
  /// \param onnx_nodes Onnx nodes from Model Proto
  /// \return The name of the unused IOs
  std::set<std::string>
  find_unused_ios(onnx::GraphProto const &onnx_graph);


  /// It will return an io_collection with the different elements of io_name if they are contained into value_infos
  /// and they are not initialized. Any initialized element in value_infos is inserted in parameters_collection
  /// \param io_names The collection of names of IO identifiers
  /// \param parameters_collection The collection of Type_info associated to the parameters elements for the given
  /// node
  /// \param value_infos The collection of IO and parameters elements
  /// \return The collection of Type_info associated to the IO elements for the given node
  io_collection_type<type_info_pointer>
  process_node_ios(google::protobuf::RepeatedPtrField<std::basic_string<char>> const &io_names,
                   io_collection_type<type_info_pointer>                             &parameters_collection,
                   Map_IO const                                                      &value_infos,
                   std::set<std::string> unused_ios = {});

  /// It will insert into onnx_io_ids the names of the elements of onnx_io
  /// \param onnx_io A collection of onnx::ValueInfoProto
  /// \param onnx_io_ids The collection of names
  void
  populate_id_collection(const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
                         std::set<std::string>                                            &onnx_io_ids);

  /// It will produce the collection of strings that are contained in
  /// onnx_io_ids and that have an element with the same name in io_collection
  /// \param onnx_io_ids The collection of IO ids
  /// \param io_collection The collection IO/parameters for the given node
  /// \return The collection of "common" names
  std::vector<std::string>
  get_common_elements(const std::set<std::string> &onnx_io_ids, io_collection_type<type_info_pointer> &io_collection);

  /// It will produce a map that associates to the tensor name of either an input, an output or a value_info that is
  /// not a parameter given by the .onnx file to a shared_ptr to Dense_tensor. Moreover, it will produce
  /// two set<string> with the names of the input and output tensors respectively
  /// \param onnx_input The inputs of a onnx::GraphProto
  /// \param onnx_output The outputs of a onnx::GraphProto
  /// \param onnx_value_info The value_infos of a onnx::GraphProto
  /// \param onnx_initializer The collection of already "known" parameters
  std::tuple<Map_IO, std::set<std::string>, std::set<std::string>>
  compute_value_infos(const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_input,
                      const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_output,
                      const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_value_info,
                      const google::protobuf::RepeatedPtrField<::onnx::TensorProto>    &onnx_initializer);

  /// It will process the attributes of the input onnx::NodeProto and produce a map that associate to the attribute
  /// name a vector of network_butcher_types::DynamicType
  /// \param node The onnx node
  /// \return The attribute map
  std::unordered_map<std::string, std::vector<network_butcher_types::DynamicType>>
  process_node_attributes(const onnx::NodeProto &node);
} // namespace network_butcher_io::Onnx_importer_helpers

#endif // NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H
