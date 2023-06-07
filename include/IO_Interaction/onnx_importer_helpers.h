#ifndef NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H
#define NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H

#include "butcher.h"
#include "graph_traits.h"
#include "parameters.h"

namespace network_butcher::io::Onnx_importer_helpers
{
  using Map_IO = std::unordered_map<std::string, Type_Info_Pointer>;

  /// Collection of helper structs (used instead of tuples to improve readability)
  namespace helpers_structures {
    /// Simple helper struct used as an output of process_node. It contains the node to be added to the graph
    struct Process_Node_Output_Type
    {
      // The node
      Converted_Onnx_Graph_Type::Node_Type node;

      // The input tensors that are inputs of the graph
      std::vector<Type_Info_Pointer> input;

      // The output tensors that are outputs of the graph
      std::vector<Type_Info_Pointer> output;
    };

    /// Simple helper struct used to store basic infos required to "decode" an onnx model
    struct Prepared_Import_Onnx_Type
    {
      Map_IO                                                value_infos;
      std::shared_ptr<network_butcher::types::Dense_tensor> pointer_input;
      std::shared_ptr<network_butcher::types::Dense_tensor> pointer_output;
      std::set<std::string>                                 onnx_inputs_ids;
      std::set<std::string>                                 onnx_outputs_ids;
    };

    /// Simple helper struct used to store basic infos required to "decode" an onnx model
    struct Processed_Value_Infos_Type
    {
      // The value infos of the graph
      Map_IO value_infos;

      // The input tensors of the graph
      std::set<std::string> onnx_inputs_ids;

      // The output tensors of the graph
      std::set<std::string> onnx_outputs_ids;
    };
  }


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

    /// It converts from a RepeatedField<T> to a vector<T>
    /// \tparam T The content type
    /// \param cont The RepeatedPtrField<T>
    /// \return The vector
    template <class T>
    auto
    converter(Repeatable_field<T> const &cont) -> std::vector<T>
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
  read_ios(Map_IO                                       &input_map,
           Repeatable_field<onnx::ValueInfoProto> const &collection,
           std::set<std::string> const                  &initialized);


  /// Inserts into the input_map the valid elements (onnx::TensorProto) contained in collection and
  /// whether they are initialized or not
  /// \param input_map The input_map
  /// \param collection The collection of IO elements
  /// \param initialized The collection of names of the initialized IO elements
  void
  read_ios(Map_IO                                    &input_map,
           Repeatable_field<onnx::TensorProto> const &collection,
           std::set<std::string> const               &initialized);


  /// It will return an io_collection with the different elements of io_names if they are contained into value_infos
  /// and they are not initialized.
  /// \param io_names The collection of names of IO identifiers
  /// \param parameters_collection The collection of Type_info associated to the parameters elements for the given
  /// node
  /// \param value_infos The collection of IO and parameters elements
  /// \return The collection of Type_info associated to the IO elements for the given node
  auto
  process_node_ios(Repeatable_field<std::basic_string<char>> const &io_names,
                   Io_Collection_Type<Type_Info_Pointer>           &parameters_collection,
                   Map_IO const &value_infos) -> Io_Collection_Type<Type_Info_Pointer>;

  /// It will insert into onnx_io_ids the names of the elements of onnx_io
  /// \param onnx_io A collection of onnx::ValueInfoProto
  /// \param onnx_io_ids The collection of names
  void
  populate_id_collection(Repeatable_field<::onnx::ValueInfoProto> const &onnx_io, std::set<std::string> &onnx_io_ids);

  /// It will produce the collection of strings that are contained in onnx_io_ids and that have an element with the
  /// same name in io_collection
  /// \param onnx_io_ids The collection of IO ids
  /// \param io_collection The collection IO/parameters for the given node
  /// \return The collection of "common" tensors
  auto
  get_common_elements(const std::set<std::string> &onnx_io_ids, Io_Collection_Type<Type_Info_Pointer> &io_collection)
    -> std::vector<Type_Info_Pointer>;


  /// It will produce a map that associates to the tensor name of either an input, an output or a value_info that is
  /// not a parameter given by the .onnx file to a shared_ptr to Dense_tensor. Moreover, it will produce the value infos
  /// and two set<string> with the names of the input and output tensors respectively
  /// \param onnx_input The inputs of a onnx::GraphProto
  /// \param onnx_output The outputs of a onnx::GraphProto
  /// \param onnx_value_info The value_infos of a onnx::GraphProto
  /// \param onnx_initializer The collection of already "known" parameters
  /// \return The map of value infos, the set of input names and the set of output names
  auto
  compute_value_infos(Repeatable_field<::onnx::ValueInfoProto> const &onnx_input,
                      Repeatable_field<::onnx::ValueInfoProto> const &onnx_output,
                      Repeatable_field<::onnx::ValueInfoProto> const &onnx_value_info,
                      Repeatable_field<::onnx::TensorProto> const    &onnx_initializer)
    -> helpers_structures::Processed_Value_Infos_Type;


  /// It will process the attributes of the input onnx::NodeProto and produce a map that associate to the attribute
  /// name a vector of network_butcher::types::DynamicType
  /// \param node The onnx node
  /// \return The attribute map
  auto
  process_node_attributes(const onnx::NodeProto &node)
    -> std::unordered_map<std::string, network_butcher::types::Variant_Attribute>;

  /// It will produce a prepared_import_onnx for the specified onnx_graph
  /// \param onnx_graph The onnx_graph
  /// \return The prepared_import_onnx struct
  auto
  prepare_import_from_onnx(onnx::GraphProto const &onnx_graph) -> helpers_structures::Prepared_Import_Onnx_Type;

  /// Simple helper method used to process a node during the graph construction
  /// \param node The specified node
  /// \param prepared_data The prepared_import_onnx struct
  /// \return The node with its inputs and outputs
  auto
  process_node(const onnx::NodeProto &node, helpers_structures::Prepared_Import_Onnx_Type const &prepared_data) -> helpers_structures::Process_Node_Output_Type;
} // namespace network_butcher::io::Onnx_importer_helpers


#endif // NETWORK_BUTCHER_ONNX_IMPORTER_HELPERS_H
