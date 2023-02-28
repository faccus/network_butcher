//
// Created by faccus on 20/02/22.
//

#ifndef NETWORK_BUTCHER_IO_MANAGER_H
#define NETWORK_BUTCHER_IO_MANAGER_H

#include "Onnx_importer_helpers.h"
#include "Onnx_model_reconstructor_helpers.h"

#if YAML_CPP_ACTIVE
#include "Yaml_importer_helpers.h"
#include <yaml-cpp/yaml.h>
#endif

#if PYBIND_ACTIVE
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#endif

#include "Computer_flops.h"
#include "GetPot"
#include "chrono.h"


namespace network_butcher_io
{
  /// IO_Manager manages the input/output interactions of the program with the local disk. The methods of this namespace
  /// allow the user to import an .onnx file, import the execution time of each network layer from a .csv file and to
  /// export a partitioned network to multiple .onnx files
  namespace IO_Manager
  {
    namespace utilities
    {
      enum Index_Type
      {
        Edge,
        Cloud,
        Operation
      };

      struct onnx_tool_output {
        std::string name;

        std::size_t macs;
        std::size_t memory;
        std::size_t params;
      };

      /// It will read from a .csv file the collection of weights for the given
      /// graph on the specified device. The .csv file must be produced by a prediction of the aMLLibrary
      /// \param graph The graph
      /// \param device The device id
      /// \param path The path of the file to be "imported"
      void
      import_weights_aMLLibrary(graph_type &graph, std::size_t device, std::string const &path);

      void
      import_weights_aMLLibrary_local(graph_type &graph, network_butcher_parameters::Parameters const& params);

      void
      csv_assembler(std::vector<std::vector<std::string>> const &content, std::string const &path);

      void
      execute_weight_generator(std::string const &csv_path,
                               std::string const &model_path,
                               std::string const &predict_path);

      /// It will read from a .csv file the collection of weights for the given
      /// graph on the specified device
      /// \param graph The graph
      /// \param device The device id
      /// \param path The path of the file to be "imported"
      void
      import_weights_custom_csv_operation_time(graph_type &graph, std::size_t device, std::string const &path);

      /// It will read from a .csv file the collection of weights for the given
      /// graph on the specified devices
      /// \param graph The graph
      /// \param devices The ids of the involved devices
      /// \param path The path of the file to be "imported"
      void
      import_weights_official_csv_multi_operation_time(graph_type              &graph,
                                                       std::vector<std::size_t> devices,
                                                       std::string const       &path);

      std::string
      aMLLibrary_generate_csv_entry(std::string const                            &entry,
                                    onnx_tool_output const                       &basic_info,
                                    graph_type::Node_Type const                  &node,
                                    const network_butcher_parameters::Parameters &params);

      std::string
      aMLLibrary_generate_csv_entry(std::string const                             &entry,
                                    graph_type::Node_Type const                   &node,
                                    const network_butcher_parameters::Parameters &params);

      /// It will read from a .csv file the collection of weights for the given
      /// graph on the specified devices
      /// \param graph The graph
      /// \param devices The ids of the involved devices
      /// \param path The path of the file to be "imported"
      void
      import_weights_custom_csv_multi_operation_time(graph_type              &graph,
                                                     std::vector<std::size_t> devices,
                                                     std::string const       &path);


      /// Based on the original graph and the partitions device/nodes, it will produce the "butchered" models and
      /// export them to the specified path (directory / name)
      /// \param partitions The partitions device/nodes
      /// \param original_model The original imported model
      /// \param link_id_nodeproto The map that associated every node of the graph to
      /// a node in the imported model
      /// \param export_base_path The export path (+ the name of the final file)
      /// \return The collection of models and the related device
      void
      reconstruct_model_and_export(
        network_butcher_types::Real_Path const     &partitions,
        onnx::ModelProto const                     &original_model,
        std::map<node_id_type, node_id_type> const &link_id_nodeproto,
        std::unordered_map<
          std::string,
          std::pair<network_butcher_io::Onnx_model_reconstructor_helpers::IO_Type,
                    std::pair<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator,
                              google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>>> const
                          &preprocessed_ios_nodes,
        const std::string &export_base_path);
    } // namespace Helper_Functions

    /// It will return the parameters read from the given file
    /// \param path The configuration file path
    /// \return The collection of parameters
    network_butcher_parameters::Parameters
    read_parameters(std::string const &path);

#if YAML_CPP_ACTIVE
    /// It will return the different Parameters read from the given .yaml files for the required models
    /// \param candidate_resources_path The candidate_resources file path
    /// \param candidate_deployments_path The candidate_deployments file path
    /// \param annotations_path The annotation file path
    /// \return The vector of Parameters
    std::vector<network_butcher_parameters::Parameters>
    read_parameters_yaml(std::string const &candidate_resources_path,
                         std::string const &candidate_deployments_path,
                         std::string const &annotations_path);
#endif

#if PYBIND_ACTIVE

    /// It will return the relative path to a .csv file containing MACs, memory usage and IO of the given model
    /// \param model_path The .onnx file path
    /// \param package_onnx_tool_location The location of the onnx_tool library
    /// \param temporary_directory The temporary directory in which the file will be stored
    /// \return The .csv file (relative) path
    std::string
    network_info_onnx_tool(std::string const &model_path,
                           std::string const &package_onnx_tool_location,
                           std::string const &temporary_directory = "tmp");



    /// It will return the relative path to a .csv file containing MACs, memory usage and IO of the given model
    /// \param params The model parameters
    /// \return The .csv file (relative) path
    std::string
    network_info_onnx_tool(network_butcher_parameters::Parameters const &params);

    std::map<std::string, utilities::onnx_tool_output>
    read_network_info_onnx_tool(std::string const &path);

#endif


    /// It will import a neural network as a graph from a given .onnx file
    /// \param path The file path of the .onnx file
    /// \param add_input_padding If true, a "fake" nodes will be added at the
    /// beginning of the network, so that the resulting graph has a single input
    /// \param add_output_padding If true, a "fake" nodes will be added at the
    /// at the end of the network, so that the resulting graph has a single output
    /// \param num_devices The number of devices
    /// \return A tuple made by the graph, the onnx::ModelProto for the .onnx file
    /// and a map associating every node in the graph to every node in the model (through their ids)
    std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
    import_from_onnx(std::string const &path,
                     bool               add_input_padding = true,
                     bool               add_output_padding = true,
                     std::size_t        num_devices       = 1,
                     bool unused_ios = false);


    /// It will export a given onnx::ModelProto to a file
    /// \param model The onnx::ModelProto
    /// \param path The export file path
    void
    export_to_onnx(onnx::ModelProto const &model, std::string path);

    /// From a given graph and the associated onnx::ModelProto, it will export the
    /// basic information about every convolutional layer in the network
    /// \param graph The graph
    /// \param model The onnx::ModelProto
    /// \param path The export file path
    void
    old_export_network_infos_to_csv(graph_type const       &graph,
                                onnx::ModelProto const &model,
                                std::string const      &path = "butcher_predict.csv");

    /// From a given graph, it will export the
    /// basic information about every convolutional layer in the network
    /// \param graph The graph
    /// \param path The export file path
    void
    export_network_infos_to_csv(graph_type const &graph, std::string const &path = "butcher_predict.csv");


    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param weight_mode The import mode (different for different .csv files)
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    void
    import_weights(network_butcher_parameters::Weight_Import_Mode const &weight_mode,
                   graph_type               &graph,
                   std::string const        &path,
                   std::size_t               device);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param weight_mode The import mode (different for different .csv files)
    /// \param graph The graph
    /// \param path The path of the file to be "imported"
    /// \param devices The device ids
    void
    import_weights(network_butcher_parameters::Weight_Import_Mode const       &weight_mode,
                   graph_type                     &graph,
                   std::string const              &path,
                   std::vector<std::size_t> const &devices);

    /// It will reconstruct the ModelProto objects associated to the different
    /// partitions and it will export them to the directory paths
    /// \param params The collection of parameters
    /// \param model The original model
    /// \param link_id_nodeproto The map that associated every node of the graph to
    /// a node in the imported model
    /// \param paths The different partitions to be exported
    void
    export_network_partitions(const network_butcher_parameters::Parameters                                 &params,
                              const onnx::ModelProto                           &model,
                              std::map<node_id_type, node_id_type> const       &link_id_nodeproto,
                              const network_butcher_types::Weighted_Real_Paths &paths);

    /// It will try to construct a onnx::ModelProto from the given partition.
    /// \param partition The partition
    /// \param original_model The original model
    /// \param link_id_nodeproto The map that associates to every node of the graph a node of the original model
    /// \param preprocessed_ios_nodes The inputs, outputs and parameters of every layer in the Model
    /// \param model_graph The graph
    /// \return A pair with a bool to represent if the operation was runned successfully and the reconstructed model
    std::pair<bool, onnx::ModelProto>
    reconstruct_model_from_partition(
      network_butcher_types::Real_Partition const &partition,
      onnx::ModelProto const                      &original_model,
      std::map<node_id_type, node_id_type> const  &link_id_nodeproto,
      std::unordered_map<
        std::string,
        std::pair<network_butcher_io::Onnx_model_reconstructor_helpers::IO_Type,
                  std::pair<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator,
                            google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>>> const
                             &preprocessed_ios_nodes,
      onnx::GraphProto const &model_graph);

  } // namespace IO_Manager

} // namespace network_butcher_io


#endif // NETWORK_BUTCHER_IO_MANAGER_H
