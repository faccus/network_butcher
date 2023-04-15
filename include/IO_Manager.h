//
// Created by faccus on 20/02/22.
//

#ifndef NETWORK_BUTCHER_IO_MANAGER_H
#define NETWORK_BUTCHER_IO_MANAGER_H

#include "Onnx_importer_helpers.h"
#include "Onnx_model_reconstructor_helpers.h"

#if YAML_CPP_ACTIVE
#  include "Yaml_importer_helpers.h"
#  include <yaml-cpp/yaml.h>
#endif

#include "GetPot"
#include "Weight_importers.h"
#include "chrono.h"


namespace network_butcher::io::IO_Manager
{
  using value_proto_collection_it  = google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator;
  using tensor_proto_collection_it = google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator;
  using preprocessed_ios_nodes_type =
    std::unordered_map<std::string,
                       std::pair<network_butcher::io::Onnx_model_reconstructor_helpers::IO_Type,
                                 std::pair<value_proto_collection_it, tensor_proto_collection_it>>>;

  namespace utilities
  {

    /// Based on the original graph and the partitions device/nodes, it will produce the "butchered" models and
    /// export them to the specified path (directory / name)
    /// \param partitions The partitions device/nodes
    /// \param original_model The original imported model
    /// \param link_id_nodeproto The map that associated every node of the graph to a node in the imported model
    /// \param preprocessed_ios_nodes The extracted input, output and parameter tensors of every layer in the Model
    /// \param export_base_path The export path (+ the name of the final file)
    void
    reconstruct_model_and_export(network_butcher::types::Real_Path const    &partitions,
                                 onnx::ModelProto const                     &original_model,
                                 std::map<node_id_type, node_id_type> const &link_id_nodeproto,
                                 preprocessed_ios_nodes_type const          &preprocessed_ios_nodes,
                                 const std::string                          &export_base_path);

  } // namespace utilities


  /// It will return the parameters read from the given file
  /// \param path The configuration file path
  /// \return The collection of parameters
  network_butcher::parameters::Parameters
  read_parameters(std::string const &path);


  /// It will import a neural network as a graph from a given .onnx file
  /// \param path The file path of the .onnx file
  /// \param add_input_padding  If true, a "fake" nodes will be added at the beginning of the network, so that
  /// the resulting graph has a single input
  /// \param add_output_padding If true, a "fake" nodes will be added at the at the end of the network, so that
  /// the resulting graph has a single output
  /// \param num_devices The number of devices
  /// \param unused_ios It will include in the searchable tensors the unused input and output tensors of the network
  /// \return A tuple made by the graph, the onnx::ModelProto for the .onnx file and a map associating every node
  /// in the graph to every node in the model (through their ids)
  std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
  import_from_onnx(std::string const &path,
                   bool               add_input_padding  = true,
                   bool               add_output_padding = true,
                   std::size_t        num_devices        = 1,
                   bool               unused_ios         = false);


  /// It will export a given onnx::ModelProto to a file
  /// \param model The onnx::ModelProto
  /// \param path The export file path
  void
  export_to_onnx(onnx::ModelProto const &model, std::string path);


  /// It will import the collection of weights for the given graph
  /// \param graph The graph
  /// \param params The parameters
  void
  import_weights(graph_type &graph, const network_butcher::parameters::Parameters &params);

  std::unique_ptr<Weight_Importer>
  generate_weight_importer(graph_type &graph, network_butcher::parameters::Parameters const &params);

  /// It will export the network partitions to multiple .onnx files
  /// \param params The parameters
  /// \param model The original model
  /// \param link_id_nodeproto The map that associates to every node of the graph a node of the original model
  /// \param paths The different partitions to be exported
  void
  export_network_partitions(const network_butcher::parameters::Parameters     &params,
                            const onnx::ModelProto                            &model,
                            std::map<node_id_type, node_id_type> const        &link_id_nodeproto,
                            const network_butcher::types::Weighted_Real_Paths &paths);


  /// It will try to construct a onnx::ModelProto from the given partition.
  /// \param partition The partition
  /// \param original_model The original model
  /// \param link_id_nodeproto The map that associates to every node of the graph a node of the original model
  /// \param preprocessed_ios_nodes The input, output and parameter tensors of every layer in the Model
  /// \param model_graph The graph
  /// \return A pair with a bool to represent if the operation was runned successfully and the reconstructed model
  std::pair<bool, onnx::ModelProto>
  reconstruct_model_from_partition(network_butcher::types::Real_Partition const &partition,
                                   onnx::ModelProto const                       &original_model,
                                   std::map<node_id_type, node_id_type> const   &link_id_nodeproto,
                                   preprocessed_ios_nodes_type const            &preprocessed_ios_nodes,
                                   onnx::GraphProto const                       &model_graph);


#if YAML_CPP_ACTIVE

  /// It will return the different Parameters read from the given .yaml files for the required models
  /// \param candidate_resources_path The candidate_resources file path
  /// \param candidate_deployments_path The candidate_deployments file path
  /// \param annotations_path The annotation file path
  /// \return The vector of Parameters
  std::vector<network_butcher::parameters::Parameters>
  read_parameters_yaml(std::string const &candidate_resources_path,
                       std::string const &candidate_deployments_path,
                       std::string const &annotations_path);

#endif

} // namespace network_butcher::io::IO_Manager

#endif // NETWORK_BUTCHER_IO_MANAGER_H
