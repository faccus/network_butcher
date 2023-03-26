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

namespace network_butcher
{
  namespace io
  {
    namespace Weight_importer_helpers
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

      /// It will read a .csv file
      /// \param path The file path
      /// \param separator The column separator character
      /// \param columns_to_read The columns to read
      /// \return The columns in a vector
      std::vector<std::vector<std::string>>
      read_csv(std::string const &path, char separator = ',', std::vector<std::string> const &columns_to_read = {});

      /// It will read a .csv file containing numbers
      /// \param path The file path
      /// \param separator The column separator character
      /// \param columns_to_read The (numeric) columns to read
      /// \return The numeric columns in a map
      std::map<std::string, std::vector<double>>
      read_csv_numerics(std::string const &path, char separator = ',', std::vector<std::string> columns_to_read = {});

      /// It will read from the specified path the relevant weights and update the graph
      /// \param graph The graph
      /// \param path The .csv file path
      /// \param devices The list of devices
      /// \param relevant_entries The name of the columns containing the relevant weights
      /// \param separator The column separator charcter
      /// \param extra_condition Update the weights of the nodes in the graph that satisfy extra_condition
      void
      import_weights_direct_read(graph_type                                               &graph,
                                 std::string const                                        &path,
                                 std::vector<std::size_t> const                           &devices,
                                 std::vector<std::string> const                           &relevant_entries,
                                 char                                                      separator       = ',',
                                 std::function<bool(graph_type::Node_Type const &)> const &extra_condition = nullptr);


      /// It will read from the specified path the relevant weights and update the graph
      /// \param graph The graph
      /// \param path The .csv file path
      /// \param devices The list of devices
      /// \param relevant_entries The name of the columns containing the relevant weights
      /// \param separator The column separator charcter
      /// \param extra_condition Update the weights of the nodes in the graph that satisfy extra_condition
      void
      import_weights_direct_read(graph_type                                               &graph,
                                 std::string const                                        &path,
                                 std::vector<network_butcher::parameters::Device> const   &devices,
                                 std::vector<std::string> const                           &relevant_entries,
                                 char                                                      separator       = ',',
                                 std::function<bool(graph_type::Node_Type const &)> const &extra_condition = nullptr);


      /// It will read from the specified path the relevant weights and update the graph
      /// \param graph The graph
      /// \param path The .csv file path
      /// \param devices The device whose weights must be imported
      /// \param relevant_entries The name of the column containing the relevant weights
      /// \param separator The column separator charcter
      /// \param extra_condition Update the weights of the nodes in the graph that satisfy extra_condition
      void
      import_weights_direct_read(graph_type                                               &graph,
                                 std::string const                                        &path,
                                 std::size_t                                               device,
                                 std::string const                                        &relevant_entry,
                                 char                                                      separator       = ',',
                                 std::function<bool(graph_type::Node_Type const &)> const &extra_condition = nullptr);

      /// It will produce a row of the aMLLibrary_prediction.csv file
      /// \param entries The entries to insert
      /// \param params The collection of parameters
      /// \param new_graph The block graph
      /// \param graph The graph
      /// \param id The node id in the block graph
      /// \param map_onnx_tool The output of onnx_tool
      /// \return The relevant row
      std::vector<std::string>
      aMLLibrary_block_generate_csv_entry(std::vector<std::string> const                &entries,
                                          network_butcher::parameters::Parameters const  &params,
                                          block_graph_type const                        &new_graph,
                                          graph_type const                              &graph,
                                          std::size_t                                    id,
                                          std::map<std::string, onnx_tool_output> const &map_onnx_tool);


      /// It will produce a specific entry of the aMLLibrary_prediction.csv file
      /// \param entry The name of the entry
      /// \param basic_info The relevant part of the onnx_tool output
      /// \param node The relevant node
      /// \param params The parameters
      /// \return The relevant entry result
      std::string
      aMLLibrary_original_generate_csv_entry(std::string const                            &entry,
                                             onnx_tool_output const                       &basic_info,
                                             graph_type::Node_Type const                  &node,
                                             const network_butcher::parameters::Parameters &params);

      /// It will produce a specific entry of the aMLLibrary_prediction.csv file (without onnx_tool)
      /// \param entry The name of the entry
      /// \param node The relevant node
      /// \param params The parameters
      /// \return The relevant entry result
      std::string
      aMLLibrary_original_generate_csv_entry(std::string const                            &entry,
                                             graph_type::Node_Type const                  &node,
                                             const network_butcher::parameters::Parameters &params);

      /// It will create a .ini file in order to use aMLLibrary
      /// \param inference_variable The inference variable
      /// \param input_path The .csv input file
      /// \param output_path The output path of the file
      void
      prepare_predict_file(std::string const &inference_variable,
                           std::string const &input_path,
                           std::string        output_path = "");


      /// It will generate and import the weights from aMLLibrary to the original graph
      /// \param graph The original graph
      /// \param params The parameters
      void
      import_weights_aMLLibrary_local_original(graph_type                                    &graph,
                                               network_butcher::parameters::Parameters const &params);


      /// It will generate and import the weights from aMLLibrary to the block graph
      /// \param new_graph The block graph
      /// \param graph The original graph
      /// \param params The parameters
      void
      import_weights_aMLLibrary_local_block(block_graph_type                              &new_graph,
                                            graph_type const                              &graph,
                                            network_butcher::parameters::Parameters const &params);


      /// It will read from a .csv file the collection of weights for the given
      /// graph on the specified device. The .csv file must be produced by a prediction of the aMLLibrary
      /// \param graph The graph
      /// \param device The device id
      /// \param path The path of the file to be "imported"
      /// \param extra_condition Update the weights of the nodes in the graph that satisfy extra_condition
      void
      import_weights_aMLLibrary_direct_read(
        graph_type                                               &graph,
        std::size_t                                               device,
        std::string const                                        &path,
        std::function<bool(graph_type::Node_Type const &)> const &extra_condition = nullptr);


      /// It will read from a .csv file the collection of weights for the given
      /// graph on the specified device. The .csv file must be produced by a prediction of the aMLLibrary
      /// \param graph The graph
      /// \param device The device id
      /// \param num_devices The number of devices
      /// \param path The path of the file to be "imported"
      /// \param extra_condition Update the weights of the nodes in the graph that satisfy extra_condition
      void
      import_weights_aMLLibrary_direct_read(
        block_graph_type                                               &graph,
        std::size_t                                                     device,
        std::size_t                                                     num_devices,
        std::string const                                              &path,
        std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition = nullptr);

      /// It will generate a .csv file from the given content in the specified path
      /// \param content The .csv content
      /// \param path The output path
      void
      csv_assembler(std::vector<std::vector<std::string>> const &content, std::string const &path);


#if PYBIND_ACTIVE

      /// It will add to the python path both the current directory and extra_packages_location
      /// \param extra_packages_location The vector of paths to add to the python path
      void
      add_python_packages(std::vector<std::string> const &extra_packages_location = {});

      /// It will launch aMLLibrary in order to generate the weights
      /// \param regressor_file The model file
      /// \param config_file The predict.ini file
      /// \param output_path The output directory
      void
      execute_weight_generator(const std::string &regressor_file,
                               const std::string &config_file,
                               const std::string &output_path);


      /// It will return the relative path to a .csv file containing MACs, memory usage and IO of the given model
      /// \param model_path The .onnx file path
      /// \param package_onnx_tool_location The location of the onnx_tool library
      /// \param temporary_directory The temporary directory in which the file will be stored
      /// \return The .csv file (relative) path
      std::string
      network_info_onnx_tool(std::string const &model_path, std::string const &temporary_directory = "tmp");


      /// It will return the relative path to a .csv file containing MACs, memory usage and IO of the given model
      /// \param params The model parameters
      /// \return The .csv file (relative) path
      std::string
      network_info_onnx_tool(network_butcher::parameters::Parameters const &params);

      /// It will read from the given path the .csv produced by onnx_tool
      /// \param path The .csv file path produced by onnx_tool
      /// \return The map containing the relevant entries and the relevant information
      std::map<std::string, onnx_tool_output>
      read_network_info_onnx_tool(std::string const &path);

#endif

    } // namespace Weight_importer_helpers

  } // namespace io
} // namespace network_butcher
#endif // NETWORK_BUTCHER_WEIGHT_IMPORTER_HELPERS_H