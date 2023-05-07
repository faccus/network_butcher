//
// Created by faccus on 8/7/22.
//

#ifndef NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H
#define NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H

#include "Butcher.h"
#include "Graph_traits.h"
#include "Parameters.h"

namespace network_butcher::io::Onnx_importer_helpers
{
  using Map_IO = std::unordered_map<std::string, type_info_pointer>;

  /// Simple helper struct used to store basic infos required to "decode" an onnx model
  struct prepared_import_onnx
  {
    Map_IO                                                value_infos;
    std::shared_ptr<network_butcher::types::Dense_tensor> pointer_input;
    std::shared_ptr<network_butcher::types::Dense_tensor> pointer_output;
    std::set<std::string>                                 onnx_inputs_ids;
    std::set<std::string>                                 onnx_outputs_ids;

    std::set<std::string> unused_ios_set;
    bool                  add_input_padding;
    bool                  add_output_padding;
  };

  namespace Utilities
  {
    /// It converts from a RepeatedField<T> to a vector<T>
    /// \tparam T The content type
    /// \param cont The RepeatedField<T>
    /// \return The vector
    template <class T>
    std::vector<T>
    converter(google::protobuf::RepeatedField<T> const &cont)
    {
      std::vector<T> res;
      res.reserve(cont.size());
      for (auto const &el : cont)
        res.push_back(el);
      return res;
    };

    /// It converts from a RepeatedField<T> to a vector<T>
    /// \tparam T The content type
    /// \param cont The RepeatedPtrField<T>
    /// \return The vector
    template <class T>
    std::vector<T>
    converter(google::protobuf::RepeatedPtrField<T> const &cont)
    {
      std::vector<T> res;
      res.reserve(cont.size());
      for (auto const &el : cont)
        res.push_back(el);
      return res;
    };

  } // namespace Utilities

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


  /// It will look for unused IOs in the onnx_graph
  /// \param onnx_nodes Onnx nodes from Model Proto
  /// \return The name of the unused IOs
  std::set<std::string>
  find_unused_ios(onnx::GraphProto const &onnx_graph);


  /// It will return an io_collection with the different elements of io_names if they are contained into value_infos
  /// and they are not initialized. Any initialized element in value_infos is inserted in parameters_collection
  /// \param io_names The collection of names of IO identifiers
  /// \param parameters_collection The collection of Type_info associated to the parameters elements for the given
  /// node
  /// \param value_infos The collection of IO and parameters elements
  /// \param unused_ios The collection of IO of the nodes that are not "used" (they either don't appear in the
  /// inputs/outputs of the graph and are not the input/output of another node)
  /// \return The collection of Type_info associated to the IO elements for the given node
  io_collection_type<type_info_pointer>
  process_node_ios(google::protobuf::RepeatedPtrField<std::basic_string<char>> const &io_names,
                   io_collection_type<type_info_pointer>                             &parameters_collection,
                   Map_IO const                                                      &value_infos,
                   std::set<std::string> const                                       &unused_ios = {});

  /// It will insert into onnx_io_ids the names of the elements of onnx_io
  /// \param onnx_io A collection of onnx::ValueInfoProto
  /// \param onnx_io_ids The collection of names
  void
  populate_id_collection(const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
                         std::set<std::string>                                            &onnx_io_ids);

  /// It will produce the collection of strings that are contained in onnx_io_ids and that have an element with the
  /// same name in io_collection
  /// \param onnx_io_ids The collection of IO ids
  /// \param io_collection The collection IO/parameters for the given node
  /// \return The collection of "common" tensors
  std::vector<type_info_pointer>
  get_common_elements(const std::set<std::string> &onnx_io_ids, io_collection_type<type_info_pointer> &io_collection);


  /// It will produce a map that associates to the tensor name of either an input, an output or a value_info that is
  /// not a parameter given by the .onnx file to a shared_ptr to Dense_tensor. Moreover, it will produce the value infos
  /// and two set<string> with the names of the input and output tensors respectively
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
  /// name a vector of network_butcher::types::DynamicType
  /// \param node The onnx node
  /// \return The attribute map
  std::unordered_map<std::string, network_butcher::types::DynamicType>
  process_node_attributes(const onnx::NodeProto &node);

  /// It will produce a prepared_import_onnx for the specified onnx_graph
  /// \param onnx_graph The onnx_graph
  /// \param add_input_padding If true, it will add an extra node at the beginning of the graph
  /// \param add_output_padding If true, it will add an extra node at the end of the graph
  /// \param unused_ios If true, we should use the unusued_ios
  /// \return The prepared_import_onnx struct
  prepared_import_onnx
  prepare_import_from_onnx(onnx::GraphProto const &onnx_graph,
                           bool                    add_input_padding,
                           bool                    add_output_padding,
                           bool                    unused_ios);

  /// Simple helper method used to process during the graph construction a node
  /// \param node The specified node
  /// \param prepared_data The prepared_import_onnx struct
  /// \return The node with its inputs and outputs
  std::tuple<node_type, std::vector<type_info_pointer>, std::vector<type_info_pointer>>
  process_node(const onnx::NodeProto &node, prepared_import_onnx const &prepared_data);
} // namespace network_butcher::io::Onnx_importer_helpers


#endif // NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H
