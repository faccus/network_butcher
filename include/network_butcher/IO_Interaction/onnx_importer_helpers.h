#ifndef NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H
#define NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H

#include <network_butcher/Butcher/butcher.h>
#include <network_butcher/Network/graph_traits.h>
#include <network_butcher/Types/parameters.h>

namespace network_butcher::io::Onnx_importer_helpers
{
  /// Simple type alias for an unordered_map of shared_ptr to Type_info, indexed by their (unique) name
  using Map_IO = std::unordered_map<std::string, Type_Info_Pointer>;

  /// Collection of helper structs (used instead of tuples to improve readability)
  namespace helpers_structures
  {
    /// Simple helper struct used as an output of process_node. It contains the node to be added to the graph
    struct Process_Node_Output_Type
    {
      /// The node
      Converted_Onnx_Graph_Type::Node_Type node;

      /// The input tensors that are inputs of the graph
      std::vector<Type_Info_Pointer> input;

      /// The output tensors that are outputs of the graph
      std::vector<Type_Info_Pointer> output;
    };

    /// Simple helper struct used to store basic infos required to "decode" an onnx model
    struct Prepared_Import_Onnx_Type
    {
      /// The collection of the value infos of the Onnx graph
      Map_IO value_infos;

      /// The input tensor of the converted graph. Padding node
      std::shared_ptr<network_butcher::types::Dense_tensor> pointer_input;

      /// The output tensor of the converted graph. Padding node
      std::shared_ptr<network_butcher::types::Dense_tensor> pointer_output;

      /// The actual input tensor names of the Onnx graph
      std::set<std::string> onnx_inputs_ids;

      /// The actual output tensor names of the Onnx graph
      std::set<std::string> onnx_outputs_ids;
    };

    /// Simple helper struct used to store basic infos required to "decode" an onnx model
    struct Processed_Value_Infos_Type
    {
      /// The value infos of the graph
      Map_IO value_infos;

      /// The input tensors of the graph
      std::set<std::string> onnx_inputs_ids;

      /// The output tensors of the graph
      std::set<std::string> onnx_outputs_ids;
    };
  } // namespace helpers_structures

  namespace Utilities
  {
    /// It converts from a RepeatedField<T> to a vector<T>
    /// \tparam T The content type
    /// \param cont The RepeatedField<T>
    /// \return The vector
    template <class T>
    auto
    converter(google::protobuf::RepeatedField<T> const &cont) -> std::vector<T>
    {
      std::vector<T> res;
      res.reserve(cont.size());
      for (auto const &el : cont)
        res.push_back(el);
      return res;
    };

    /// It converts from a RepeatablePtr_field<T> to a vector<T>
    /// \tparam T The content type
    /// \param cont The RepeatedPtrField<T>
    /// \return The vector
    template <class T>
    auto
    converter(RepeatablePtr_field<T> const &cont) -> std::vector<T>
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
  /// \param extra_initialized_condition Should an initializer also be a tensor that is not part of the second collection?
  /// \param extra_non_initialized_condition It contains the name of the tensors that are NOT initialized. If a tensor is
  /// not in this collection, then it is considered as an initializer (and, thus, as a node parameter)
  void
  read_ios(Map_IO                                          &input_map,
           RepeatablePtr_field<onnx::ValueInfoProto> const &collection,
           std::set<std::string> const                     &initialized,
           bool                                             extra_initialized_condition     = false,
           std::set<std::string> const                     &extra_non_initialized_condition = {});


  /// Inserts into the input_map the valid elements (onnx::TensorProto) contained in collection and
  /// whether they are initialized or not
  /// \param input_map The input_map
  /// \param collection The collection of IO elements
  /// \param initialized The collection of names of the initialized IO elements
  void
  read_ios(Map_IO                                       &input_map,
           RepeatablePtr_field<onnx::TensorProto> const &collection,
           std::set<std::string> const                  &initialized);


  /// It will return an io_collection with the different elements of io_names that are contained into value_infos
  /// and that are not initialized.
  /// \param io_names The collection of names of IO identifiers
  /// \param parameters_collection The collection of Type_info pointers associated to the parameters elements for the given
  /// node
  /// \param value_infos The collection of IO and parameters elements of the graph
  /// \return The collection of Type_info associated to the IO elements for the given node
  auto
  process_node_ios(RepeatablePtr_field<std::basic_string<char>> const &io_names,
                   Io_Collection_Type<Type_Info_Pointer>              &parameters_collection,
                   Map_IO const &value_infos) -> Io_Collection_Type<Type_Info_Pointer>;

  /// It will insert into onnx_io_ids the names of the elements of onnx_io
  /// \param onnx_io A collection of onnx::ValueInfoProto
  /// \param onnx_io_ids The collection of names
  void
  populate_id_collection(RepeatablePtr_field<::onnx::ValueInfoProto> const &onnx_io,
                         std::set<std::string>                             &onnx_io_ids);

  /// It will produce the collection of strings that are contained in onnx_io_ids and that have an element with the
  /// same name in io_collection
  /// \param onnx_io_ids The collection of IO ids
  /// \param io_collection The collection IO/parameters for the given node
  /// \return The collection of "common" tensors
  auto
  get_common_elements(const std::set<std::string> &onnx_io_ids, Io_Collection_Type<Type_Info_Pointer> &io_collection)
    -> std::vector<Type_Info_Pointer>;


  /// It will produce a map that associates to the tensor name of either an input, an output or a value_info (a
  /// non-input and non-output tensor) to a shared_ptr to Dense_tensor. Moreover, it will produce the value infos and
  /// two set<string> with the names of the input and output tensors respectively
  /// \param onnx_graph The onnx_graph
  /// \return The map of value infos (containing inputs, outputs and value_infos), the set of input names and the set of
  /// output names
  auto
  compute_value_infos(onnx::GraphProto const &onnx_graph)
    -> helpers_structures::Processed_Value_Infos_Type;


  /// It will process the attributes of the input onnx::NodeProto and produce a map that associates to the name of the
  /// attribute a Variant_Attribute (either a vector of ints, floats or strings)
  /// \param node The onnx node
  /// \return The attribute map
  auto
  process_node_attributes(const onnx::NodeProto &node)
    -> std::unordered_map<std::string, network_butcher::types::Variant_Attribute>;

  /// It will pre-process the Onnx graph to speed up the conversion from Onnx graph to Network Butcher graph
  /// \param onnx_graph The onnx_graph
  /// \return The prepared_import_onnx struct
  auto
  prepare_import_from_onnx(onnx::GraphProto const &onnx_graph) -> helpers_structures::Prepared_Import_Onnx_Type;

  /// Simple helper method used to process a node during the graph conversion
  /// \param node The specified node
  /// \param prepared_data The Prepared_Import_Onnx_Type struct
  /// \return The node with its inputs and outputs
  auto
  process_node(const onnx::NodeProto &node, helpers_structures::Prepared_Import_Onnx_Type const &prepared_data)
    -> helpers_structures::Process_Node_Output_Type;
} // namespace network_butcher::io::Onnx_importer_helpers


#endif // NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H