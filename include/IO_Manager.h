//
// Created by faccus on 20/02/22.
//

#ifndef NETWORK_BUTCHER_IO_MANAGER_H
#define NETWORK_BUTCHER_IO_MANAGER_H

#include "IO_Interaction/Onnx_importer_helpers.h"
#include "IO_Interaction/Onnx_model_reconstructor_helpers.h"

#if YAML_CPP_ACTIVE
#include "IO_Interaction/Yaml_importer_helpers.h"
#include <yaml-cpp/yaml.h>
#endif

#include "APSC/chrono.h"
#include "APSC/GetPot"


namespace network_butcher_io
{
  class IO_Manager
  {
  private:
    enum Index_Type
    {
      Edge,
      Cloud,
      Operation
    };

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device. The .csv file must be produced by a prediction of the aMLLibrary
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights_aMLLibrary(graph_type &graph, std::size_t device, std::string const &path);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights_custom_csv_operation_time(graph_type &graph, std::size_t device, std::string const &path);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified devices
    /// \param graph The graph
    /// \param devices The ids of the involved devices
    /// \param path The path of the file to be "imported"
    static void
    import_weights_official_csv_multi_operation_time(graph_type              &graph,
                                                     std::vector<std::size_t> devices,
                                                     std::string const       &path);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified devices
    /// \param graph The graph
    /// \param devices The ids of the involved devices
    /// \param path The path of the file to be "imported"
    static void
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
    static void
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

  public:
    /// It will return the parameters read from the given file
    /// \param path The configuration file path
    /// \return The collection of parameters
    static Parameters
    read_parameters(std::string const &path);

#if YAML_CPP_ACTIVE
    /// It will return the different Parameters read from the given .yaml files for the required models
    /// \param candidate_resources_path The candidate_resources file path
    /// \param candidate_deployments_path The candidate_deployments file path
    /// \param annotations_path The annotation file path
    /// \return The vector of Parameters
    static std::vector<Parameters>
    read_parameters_yaml(std::string const &candidate_resources_path,
                         std::string const &candidate_deployments_path,
                         std::string const &annotations_path);
#endif

    /// It will import a neural network as a graph from a given .onnx file
    /// \param path The file path of the .onnx file
    /// \param add_padding_nodes If true, two "fake" nodes will be added at the
    /// beginning of the network and at the end, so that the resulting graph has
    /// a single input and a single output
    /// \param num_devices The number of devices
    /// \return A tuple made by the graph, the onnx::ModelProto for the .onnx file
    /// and a map associating every node in the graph to every node in the model (through their ids)
    static std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
    import_from_onnx(std::string const &path, bool add_padding_nodes = true, std::size_t num_devices = 1);


    /// It will export a given onnx::ModelProto to a file
    /// \param model The onnx::ModelProto
    /// \param path The export file path
    static void
    export_to_onnx(onnx::ModelProto const &model, std::string path);

    /// From a given graph and the associated onnx::ModelProto, it will export the
    /// basic information about every convolutional layer in the network
    /// \param graph The graph
    /// \param model The onnx::ModelProto
    /// \param path The export file path
    static void
    export_network_infos_to_csv(graph_type const       &graph,
                                onnx::ModelProto const &model,
                                std::string const      &path = "butcher_predict.csv");


    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param weight_mode The import mode (different for different .csv files)
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights(Weight_Import_Mode const &weight_mode,
                   graph_type               &graph,
                   std::string const        &path,
                   std::size_t               device);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param weight_mode The import mode (different for different .csv files)
    /// \param graph The graph
    /// \param path The path of the file to be "imported"
    /// \param devices The device ids
    static void
    import_weights(Weight_Import_Mode const       &weight_mode,
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
    static void
    export_network_partitions(const Parameters                                 &params,
                              const onnx::ModelProto                           &model,
                              std::map<node_id_type, node_id_type> const       &link_id_nodeproto,
                              const network_butcher_types::Weighted_Real_Paths &paths);


    static std::pair<bool, onnx::ModelProto>
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
  };

} // namespace network_butcher_io


#endif // NETWORK_BUTCHER_IO_MANAGER_H
