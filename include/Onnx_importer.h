//
// Created by faccus on 8/7/22.
//

#ifndef NETWORK_BUTCHER_ONNX_IMPORTER_H
#define NETWORK_BUTCHER_ONNX_IMPORTER_H

#include "Butcher.h"
#include "Traits/Graph_traits.h"
#include "Types/Parameters.h"


namespace network_butcher_io
{

  class Onnx_importer
  {
  private:
    using Map_IO = std::unordered_map<std::string, type_info_pointer>;

    /// Inserts into the input_map the valid elements (onnx::ValueInfoProto) contained in collection and
    /// whether they are initialized or not
    /// \param input_map The input_map
    /// \param collection The collection of IO elements
    /// \param initialized The collection of names of the initialized IO elements
    static void
    read_ios(Map_IO                                                         &input_map,
             google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> const &collection,
             std::set<std::string> const                                    &initialized);

    /// Inserts into the input_map the valid elements (onnx::TensorProto) contained in collection and
    /// whether they are initialized or not
    /// \param input_map The input_map
    /// \param collection The collection of IO elements
    /// \param initialized The collection of names of the initialized IO elements
    static void
    read_ios(Map_IO                                                      &input_map,
             google::protobuf::RepeatedPtrField<onnx::TensorProto> const &collection,
             std::set<std::string> const                                 &initialized);

    /// It will return an io_collection with the different elements of io_name if they are contained into value_infos
    /// and they are not initialized. Any initialized element in value_infos is inserted in parameters_collection
    /// \param io_names The collection of names of IO identifiers
    /// \param parameters_collection The collection of Type_info associated to the parameters elements for the given
    /// node
    /// \param value_infos The collection of IO and parameters elements
    /// \return The collection of Type_info associated to the IO elements for the given node
    static io_collection_type<type_info_pointer>
    process_node_ios(google::protobuf::RepeatedPtrField<std::basic_string<char>> const &io_names,
                     io_collection_type<type_info_pointer>                             &parameters_collection,
                     Map_IO const                                                      &value_infos);

    /// It will insert into onnx_io_ids the names of the elements of onnx_io
    /// \param onnx_io A collection of onnx::ValueInfoProto
    /// \param onnx_io_ids The collection of names
    static void
    populate_id_collection(const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
                           std::set<std::string>                                            &onnx_io_ids);

    /// It will produce the collection of strings that are contained in
    /// onnx_io_ids and that have an element with the same name in io_collection
    /// \param onnx_io_ids The collection of IO ids
    /// \param io_collection The collection IO/parameters for the given node
    /// \return The collection of "common" names
    static std::vector<std::string>
    get_common_elements(const std::set<std::string> &onnx_io_ids, io_collection_type<type_info_pointer> &io_collection);

    /// It will produce a map that associates to the tensor name of either an input, an output or a value_info that is
    /// not a parameter given by the .onnx file to a shared_ptr to Dense_tensor. Moreover, it will produce
    /// two set<string> with the names of the input and output tensors respectively
    /// \param onnx_input The inputs of a onnx::GraphProto
    /// \param onnx_output The outputs of a onnx::GraphProto
    /// \param onnx_value_info The value_infos of a onnx::GraphProto
    /// \param onnx_initializer The collection of already "known" parameters
    static std::tuple<Map_IO, std::set<std::string>, std::set<std::string>>
    compute_value_infos(const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_input,
                        const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_output,
                        const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_value_info,
                        const google::protobuf::RepeatedPtrField<::onnx::TensorProto>    &onnx_initializer);

    /// It will process the attributes of the input onnx::NodeProto and produce a map that associate to the attribute
    /// name a vector of network_butcher_types::DynamicType
    /// \param node The onnx node
    /// \return The attribute map
    static std::unordered_map<std::string, std::vector<network_butcher_types::DynamicType>>
    process_node_attributes(const onnx::NodeProto &node);

  public:
    /// It will import a neural network as a graph from a given .onnx file
    /// \param path The file path of the .onnx file
    /// \param add_padding_nodes If true, two "fake" nodes will be added at the
    /// beginning of the network and at the end, so that the resulting graph has
    /// a single input and a single output
    /// \param num_devices The number of devices
    /// \return A tuple made by the graph, the onnx::ModelProto for the .onnx file
    /// and a map associating every node in the graph to every node in the model
    static std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
    import_from_onnx(std::string const &path, bool add_padding_nodes, std::size_t num_devices);
  };
} // namespace network_butcher_io

#endif // NETWORK_BUTCHER_ONNX_IMPORTER_H
