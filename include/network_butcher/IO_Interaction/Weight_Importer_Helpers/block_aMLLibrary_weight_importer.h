#ifndef NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H

#if NETWORK_BUTCHER_PYBIND_ACTIVE
#  include <network_butcher/Extra/CMake_directives.h>
#  include <pybind11/embed.h>
#  include <pybind11/pybind11.h>

#  if PLATFORM_SPECIFIC_CONFIG
#    include <network_butcher/Extra/platform_specific_config.h>
#  endif
#endif

#include <cmath>

#include <network_butcher/Computer/computer_memory.h>
#include <network_butcher/IO_Interaction/Weight_Importer_Helpers/weight_importer.h>
#include <network_butcher/IO_Interaction/Weight_Importer_Helpers/weight_importer_utils.h>

#include <network_butcher/IO_Interaction/Weight_Importer_Helpers/csv_weight_importer.h>

namespace network_butcher::io
{
  /// This class will be used to generate and import weights with aMLLibrary into the block graph
  class block_aMLLibrary_Weight_Importer : public Weight_Importer<Block_Graph_Type>
  {
  protected:
    /// The collection of block graph related parameters
    network_butcher::parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params;

    /// The collection of aMLLibrary related parameters
    network_butcher::parameters::Parameters::aMLLibrary const &aMLLibrary_params;

    /// The collection of weights related parameters
    parameters::Parameters::Weights const &weights_params;

    /// The collection of model related parameters
    network_butcher::parameters::Parameters::Model const &model_params;

    /// The collection of devices
    parameters::Parameters::Devices const &devices;

    /// The original graph
    Converted_Onnx_Graph_Type const &original_graph;

    /// The block graph
    using Weight_Importer<Block_Graph_Type>::graph;


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
    /// \param regressor_file The regressor model path
    /// \param config_file The configuration file
    /// \param output_path The output path
    void
    execute_weight_generator(const std::string &regressor_file,
                             const std::string &config_file,
                             const std::string &output_path) const;


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
    /// \param content The content of the .csv file
    /// \param path The path of the .csv file
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
    /// \param node_output_ids The ids of the output nodes of the node in the original graph
    /// \return The relevant entry in the .csv file
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
      Converted_Onnx_Graph_Type const                                       &original_graph,
      Block_Graph_Type                                                      &graph,
      network_butcher::parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params,
      network_butcher::parameters::Parameters::aMLLibrary const             &aMLLibrary_params,
      parameters::Parameters::Weights const                                 &weights_params,
      network_butcher::parameters::Parameters::Model const                  &model_params,
      parameters::Parameters::Devices const                                 &devices)
      : Weight_Importer(graph)
      , original_graph{original_graph}
      , block_graph_generation_params{block_graph_generation_params}
      , aMLLibrary_params{aMLLibrary_params}
      , weights_params{weights_params}
      , model_params{model_params}
      , devices{devices}
    {
      check_aMLLibrary();
    };

    block_aMLLibrary_Weight_Importer(Converted_Onnx_Graph_Type const               &original_graph,
                                     Block_Graph_Type                              &graph,
                                     network_butcher::parameters::Parameters const &params)
      : block_aMLLibrary_Weight_Importer(original_graph,
                                         graph,
                                         params.block_graph_generation_params,
                                         params.aMLLibrary_params,
                                         params.weights_params,
                                         params.model_params,
                                         params.devices){};


    /// It will import the weights into the block graph
    void
    import_weights() override;

    ~block_aMLLibrary_Weight_Importer() override = default;
  };

} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H