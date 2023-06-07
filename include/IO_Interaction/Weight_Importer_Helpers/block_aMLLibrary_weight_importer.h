#ifndef NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H

#if PYBIND_ACTIVE
#  include "CMake_directives.h"
#  include <pybind11/embed.h>
#  include <pybind11/pybind11.h>

#  if PLATFORM_SPECIFIC_CONFIG
#    include "platform_specific_config.h"
#  endif
#endif

#include "computer_memory.h"
#include "weight_importer.h"
#include "weight_importer_utils.h"

#include "csv_weight_importer.h"

namespace network_butcher::io
{
  /// This class will be used to generate and import weights with aMLLibrary into the block graph
  class block_aMLLibrary_Weight_Importer : public Weight_Importer
  {
  protected:
    /// Collection of parameters
    network_butcher::parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params;
    network_butcher::parameters::Parameters::aMLLibrary const             &aMLLibrary_params;
    parameters::Parameters::Weights const                                 &weights_params;
    network_butcher::parameters::Parameters::Model const                  &model_params;
    parameters::Parameters::Devices const                                 &devices;

    Converted_Onnx_Graph_Type const &graph;
    Block_Graph_Type                &new_graph;

    /// It will check if the aMLLibrary is available
    void
    check_aMLLibrary() const;


    /// It will add the python packages to the python path
    void
    add_python_packages() const;


    /// It will create a .ini file in order to use aMLLibrary
    /// \param inference_variable The inference variable
    /// \param input_path The .csv input file
    /// \param output_path The output path of the file
    static void
    prepare_predict_file(std::string const &inference_variable,
                         std::string const &input_path,
                         std::string        output_path = "");


    /// It will execute the weight generator
    /// \param regressor_file The regressor file
    /// \param config_file The configuration file
    /// \param output_path The output path
    static void
    execute_weight_generator(const std::string &regressor_file,
                             const std::string &config_file,
                             const std::string &output_path);


    /// It will execute onnx_tool in order to obtain the network information
    /// \return The path of the .csv file containing the network information
    [[nodiscard]] auto
    network_info_onnx_tool() const -> std::string;


    /// It will read the network information from the .csv file
    /// \param path The path of the .csv file
    /// \return A map containing the network information
    [[nodiscard]] static auto
    read_network_info_onnx_tool(const std::string &path)
      -> std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type>;


    /// It will prepare and export the .csv file to be fed to aMLLibrary
    static void
    csv_assembler(const std::vector<std::vector<std::string>> &content, const std::string &path);


    /// It will start aMMLibrary to produce the .csv files for the various devices
    /// \param csv_path The path containing the .csv file used as an input of aMMLibrary
    /// \return The pair containing the paths and the relevant entries
    [[nodiscard]] auto
    perform_predictions(std::string const &csv_path) const
      -> std::pair<std::vector<std::string>, std::vector<std::string>>;


    /// It will generate the relevant entry given its name and the node
    /// \param lower_case The entry name in lower case
    /// \param id The node id in the block graph
    /// \param map_onnx_tool The output of onnx_tool
    /// \param previous_entries_info The info of the previous entries elements in the .csv
    /// \param original_ids The ids of the original graph of the node
    [[nodiscard]] auto
    generate_entry(std::string const                                                           &lower_case,
                   std::size_t                                                                  id,
                   std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type> const &map_onnx_tool,
                   std::map<std::string, std::size_t>                                          &previous_entries_info,
                   Node_Id_Collection_Type const                                               &original_ids,
                   Node_Id_Collection_Type const &node_output_ids) const -> std::string;


    /// It will produce a row of the aMLLibrary_prediction.csv file
    /// \param entries The entries to insert
    /// \param id The node id in the block graph
    /// \param map_onnx_tool The output of onnx_tool
    /// \return The relevant row in the .csv file
    [[nodiscard]] auto
    generate_entries(std::vector<std::string> const                                              &entries,
                     std::size_t                                                                  id,
                     std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type> const &map_onnx_tool) const
      -> std::vector<std::string>;


  public:
    block_aMLLibrary_Weight_Importer(
      Converted_Onnx_Graph_Type const                                       &graph,
      Block_Graph_Type                                                      &new_graph,
      network_butcher::parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params,
      network_butcher::parameters::Parameters::aMLLibrary const             &aMLLibrary_params,
      parameters::Parameters::Weights const                                 &weights_params,
      network_butcher::parameters::Parameters::Model const                  &model_params,
      parameters::Parameters::Devices const                                 &devices)
      : Weight_Importer()
      , block_graph_generation_params{block_graph_generation_params}
      , aMLLibrary_params{aMLLibrary_params}
      , weights_params{weights_params}
      , model_params{model_params}
      , devices{devices}
      , graph{graph}
      , new_graph{new_graph}
    {
      check_aMLLibrary();
    };

    block_aMLLibrary_Weight_Importer(Converted_Onnx_Graph_Type const               &graph,
                                     Block_Graph_Type                              &new_graph,
                                     network_butcher::parameters::Parameters const &params)
      : block_aMLLibrary_Weight_Importer(graph,
                                         new_graph,
                                         params.block_graph_generation_params,
                                         params.aMLLibrary_params,
                                         params.weights_params,
                                         params.model_params,
                                         params.devices){};


    /// It will import the weights into the block graph
    void
    import_weights() override;

    /// It will import the weights into the block graph
    /// \param extra_condition A function that will be used to filter the nodes to import (it will skip the nodes that
    /// do not satisfy the condition)
    void
    import_weights(std::function<bool(Block_Graph_Type::Node_Type const &)> const &extra_condition);

    virtual ~block_aMLLibrary_Weight_Importer() = default;
  };

} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H
