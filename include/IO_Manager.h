//
// Created by faccus on 20/02/22.
//

#ifndef NETWORK_BUTCHER_IO_MANAGER_H
#define NETWORK_BUTCHER_IO_MANAGER_H

#include "Onnx_interaction/Onnx_importer_helpers.h"
#include "Onnx_interaction/Onnx_model_reconstructor_helpers.h"

#include "APSC/GetPot"

namespace network_butcher_io
{
  class IO_Manager
  {
  private:
    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights_from_csv_aMLLibrary(graph_type &graph, std::size_t device, std::string const &path);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights_from_csv_operation_time(graph_type &graph, std::size_t device, std::string const &path);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights_from_csv_multi_operation_time(graph_type              &graph,
                                                 std::vector<std::size_t> device,
                                                 std::string const       &path);


  public:
    /// It will return the parameters read from the given file
    /// \param path The configuration file path
    /// \return The collection of parameters
    static Parameters
    read_parameters(std::string const &path);

    /// It will import a neural network as a graph from a given .onnx file
    /// \param path The file path of the .onnx file
    /// \param add_padding_nodes If true, two "fake" nodes will be added at the
    /// beginning of the network and at the end, so that the resulting graph has
    /// a single input and a single output
    /// \param num_devices The number of devices
    /// \return A tuple made by the graph, the onnx::ModelProto for the .onnx file
    /// and a map associating every node in the graph to every node in the model
    static std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
    import_from_onnx(std::string const &path, bool add_padding_nodes = true, std::size_t num_devices = 1);

    /// It will export a given onnx::ModelProto to a file
    /// \param model The onnx::ModelProto
    /// \param path The export file path
    static void
    export_to_onnx(onnx::ModelProto const &model, std::string path);

    /// From a given graph and the associated onnx::ModelProto, it will export the
    /// basic information about every convolutional node in the network
    /// \param graph The graph
    /// \param model The onnx::ModelProto
    /// \param path The export file path
    static void
    export_network_infos_to_csv(graph_type const       &graph,
                                onnx::ModelProto const &model,
                                std::string const      &path = "butcher_predict.csv");


    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param weight_mode The type of .csv file to be imported
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
    /// \param weight_mode The type of .csv file to be imported
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights(Weight_Import_Mode const &weight_mode,
                   graph_type               &graph,
                   std::string const        &path,
                   std::vector<std::size_t>  devices,
                   std::size_t               index = 0);

    /// Based on the graph and the partitions device/nodes, it will prodice the
    /// "butchered" models.
    /// \tparam Graph The type of the graph
    /// \param partitions The partitions device/nodes
    /// \param original_model The original imported model
    /// \param graph The graph
    /// \param link_id_nodeproto The map that associated every node of the graph to
    /// a node in the imported model
    /// \return The collection of models and the related device
    template <class Graph>
    static std::vector<std::pair<onnx::ModelProto, std::size_t>>
    reconstruct_model(network_butcher_types::Real_Path const     &partitions,
                      onnx::ModelProto const                     &original_model,
                      Graph const                                &graph,
                      std::map<node_id_type, node_id_type> const &link_id_nodeproto);

    /// It will reconstruct the ModelProto objects associated to the different
    /// partitions and it will export them to the directory paths
    /// \param params The collection of parameters
    /// \param graph The graph
    /// \param model The original model
    /// \param link_id_nodeproto The map that associated every node of the graph to
    /// a node in the imported model
    /// \param paths The different partitions to be exported
    static void
    export_network_partitions(const Parameters                                 &params,
                              const graph_type                                 &graph,
                              const onnx::ModelProto                           &model,
                              std::map<node_id_type, node_id_type> const       &link_id_nodeproto,
                              const network_butcher_types::Weighted_Real_Paths &paths);
  };

  template <class Graph>
  std::vector<std::pair<onnx::ModelProto, std::size_t>>
  network_butcher_io::IO_Manager::reconstruct_model(const network_butcher_types::Real_Path     &partitions,
                                                    const onnx::ModelProto                     &original_model,
                                                    const Graph                                &graph,
                                                    const std::map<node_id_type, node_id_type> &link_id_nodeproto)
  {
    std::vector<std::pair<onnx::ModelProto, std::size_t>> res;

    auto const &model_graph = original_model.graph();

    for (const auto &partition : partitions)
      {
        auto const &device_id = partition.first;
        auto const &node_ids  = partition.second;

        res.emplace_back(onnx::ModelProto(), device_id);

        Onnx_model_reconstructor_helpers::prepare_new_model(original_model, res.back().first);

        auto current_edited_graph = Onnx_model_reconstructor_helpers::prepare_new_graph(original_model);

        Onnx_model_reconstructor_helpers::add_nodes(link_id_nodeproto, model_graph, node_ids, current_edited_graph);

        Onnx_model_reconstructor_helpers::add_missing_inputs(original_model, current_edited_graph);
        Onnx_model_reconstructor_helpers::add_missing_outputs(original_model, current_edited_graph);

        res.back().first.set_allocated_graph(current_edited_graph);
      }

    return res;
  }

} // namespace network_butcher_io


#endif // NETWORK_BUTCHER_IO_MANAGER_H
