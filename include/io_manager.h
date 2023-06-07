#ifndef NETWORK_BUTCHER_IO_MANAGER_H
#define NETWORK_BUTCHER_IO_MANAGER_H

#include <ranges>

#include "onnx_importer_helpers.h"
#include "onnx_model_reconstructor_helpers.h"

#include "GetPot"
#include "chrono.h"
#include "weight_importers.h"


namespace network_butcher::io::IO_Manager
{
  namespace utilities
  {
    /// It will generate the Weight_Importer for the given graph
    /// \param graph The graph
    /// \param params The parameters
    /// \return The Weight_Importer
    auto
    generate_weight_importer(Converted_Onnx_Graph_Type &graph, network_butcher::parameters::Parameters const &params)
      -> std::unique_ptr<Weight_Importer>;

    /// Based on the original graph and the partitions device/nodes, it will produce the "butchered" models and
    /// export them to the specified path (directory / name)
    /// \param weighted_path The partitions device/nodes and the overall length
    /// \param original_model The original imported model
    /// \param link_id_nodeproto The map that associated every node of the graph to a node in the imported model
    /// \param preprocessed_ios_nodes The extracted input, output and parameter tensors of every layer in the Model
    /// \param export_base_path The export path (+ the name of the final file)
    void
    reconstruct_model_and_export(
      network_butcher::types::Weighted_Real_Path const                                 &weighted_path,
      onnx::ModelProto const                                                           &original_model,
      std::map<Node_Id_Type, Node_Id_Type> const                                       &link_id_nodeproto,
      Onnx_model_reconstructor_helpers::helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes,
      const std::string                                                                &export_base_path);

  } // namespace utilities

  /// It will return the parameters read from the given file
  /// \param path The configuration file path
  /// \return The collection of parameters
  auto
  read_parameters(std::string const &path) -> network_butcher::parameters::Parameters;


  /// It will import a neural network as a graph from a given .onnx file
  /// \param path The file path of the .onnx file
  /// \param add_input_padding  If true, a padding node will be added at the beginning of the network, so that
  /// the resulting graph has a single input
  /// \param add_output_padding If true, a padding nodes will be added at the at the end of the network, so that
  /// the resulting graph has a single output
  /// \param num_devices The number of devices
  /// \return A tuple made by the graph, the onnx::ModelProto for the .onnx file and a map associating every node
  /// in the graph to every node in the model (through their ids)
  auto
  import_from_onnx(std::string const &path,
                   bool               add_input_padding  = true,
                   bool               add_output_padding = true,
                   std::size_t        num_devices        = 1)
    -> std::tuple<Converted_Onnx_Graph_Type, onnx::ModelProto, std::map<Node_Id_Type, Node_Id_Type>>;


  /// It will export a given onnx::ModelProto to a file
  /// \param model The onnx::ModelProto
  /// \param path The export file path
  void
  export_to_onnx(onnx::ModelProto const &model, const std::string& path);


  /// It will import the collection of weights for the given graph (if the the proper method was chosen). Otherwise,
  /// it will not do anything
  /// \param graph The graph
  /// \param params The parameters
  void
  import_weights(Converted_Onnx_Graph_Type &graph, const network_butcher::parameters::Parameters &params);


  /// It will export the network partitions to multiple .onnx files
  /// \param params The parameters
  /// \param model The original model
  /// \param link_id_nodeproto The map that associates to every node of the graph a node of the original model
  /// \param paths The different partitions to be exported
  void
  export_network_partitions(const network_butcher::parameters::Parameters                 &params,
                            const onnx::ModelProto                                        &model,
                            std::map<Node_Id_Type, Node_Id_Type> const                    &link_id_nodeproto,
                            const std::vector<network_butcher::types::Weighted_Real_Path> &paths);


  /// It will try to construct a onnx::ModelProto from the given partition.
  /// \param partition The partition
  /// \param original_model The original model
  /// \param link_id_nodeproto The map that associates to every node of the graph a node of the original model
  /// \param preprocessed_ios_nodes The input, output and parameter tensors of every layer in the Model
  /// \param model_graph The graph
  /// \return A pair with a bool to represent if the operation was run successfully and the reconstructed model
  auto
  reconstruct_model_from_partition(
    network_butcher::types::Real_Partition const                                     &partition,
    onnx::ModelProto const                                                           &original_model,
    std::map<Node_Id_Type, Node_Id_Type> const                                       &link_id_nodeproto,
    Onnx_model_reconstructor_helpers::helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes,
    onnx::GraphProto const &model_graph) -> std::pair<bool, onnx::ModelProto>;
} // namespace network_butcher::io::IO_Manager

#endif // NETWORK_BUTCHER_IO_MANAGER_H
