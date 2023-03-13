#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_HELPERS_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_HELPERS_H


#include "Basic_traits.h"
#include "Computer_memory.h"
#include "Graph_traits.h"
#include "Parameters.h"

#if PYBIND_ACTIVE
#  include "CMake_directives.h"
#  include <pybind11/embed.h>
#  include <pybind11/pybind11.h>
#endif

namespace network_butcher_io::Weight_importer_helpers
{
  enum Index_Type
  {
    Edge,
    Cloud,
    Operation
  };

  struct onnx_tool_output
  {
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
  import_weights_aMLLibrary_direct_read(
    graph_type                                               &graph,
    std::size_t                                               device,
    std::string const                                        &path,
    std::function<bool(graph_type::Node_Type const &)> const &extra_condition = nullptr);

  void
  import_weights_aMLLibrary_direct_read(
    block_graph_type                                               &graph,
    std::size_t                                                     device,
    std::size_t                                                     num_devices,
    std::string const                                              &path,
    std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition = nullptr);

  void
  import_weights_aMLLibrary_local_original(graph_type &graph, network_butcher_parameters::Parameters const &params);

  void
  csv_assembler(std::vector<std::vector<std::string>> const &content, std::string const &path);

  void
  import_weights_aMLLibrary_local_block(block_graph_type                             &new_graph,
                                        graph_type const                             &graph,
                                        network_butcher_parameters::Parameters const &params);


#if PYBIND_ACTIVE

  void
  add_aMLLibrary_to_path();

  void
  execute_weight_generator(const std::string &regressor_file,
                           const std::string &config_file,
                           const std::string &output_path,
                           const std::string &package_path);

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

  std::map<std::string, onnx_tool_output>
  read_network_info_onnx_tool(std::string const &path);

#endif

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


  std::vector<std::string>
  aMLLibrary_block_generate_csv_entry(std::vector<std::string> const                &entries,
                                      network_butcher_parameters::Parameters const  &params,
                                      block_graph_type const                        &new_graph,
                                      graph_type const                              &graph,
                                      std::size_t                                    id,
                                      std::map<std::string, onnx_tool_output> const &map_onnx_tool);


  std::string
  aMLLibrary_original_generate_csv_entry(std::string const                            &entry,
                                         onnx_tool_output const                       &basic_info,
                                         graph_type::Node_Type const                  &node,
                                         const network_butcher_parameters::Parameters &params);

  std::string
  aMLLibrary_original_generate_csv_entry(std::string const                            &entry,
                                         graph_type::Node_Type const                  &node,
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


} // namespace network_butcher_io::Weight_importer_helpers

#endif // NETWORK_BUTCHER_WEIGHT_IMPORTER_HELPERS_H