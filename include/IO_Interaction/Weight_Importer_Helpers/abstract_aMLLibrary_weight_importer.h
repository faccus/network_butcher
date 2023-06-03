//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_BASIC_AMLLIBRARY_WEIGHT_IMPOTER_H
#define NETWORK_BUTCHER_BASIC_AMLLIBRARY_WEIGHT_IMPOTER_H

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
  /// This (pure virtual) class will be used to define common utilities required to import weights with aMLLibrary
  class Abstract_aMLLibrary_Weight_Importer : public Weight_Importer
  {
  protected:
    /// Collection of parameters
    network_butcher::parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params;
    network_butcher::parameters::Parameters::aMLLibrary const             &aMLLibrary_params;
    parameters::Parameters::Weights const                                 &weights_params;
    network_butcher::parameters::Parameters::Model const                  &model_params;
    parameters::Parameters::Devices const                                 &devices;

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
      -> std::map<std::string, Weight_importer_helpers::onnx_tool_output>;

    /// It will prepare the .csv file to be fed to aMLLibrary
    static void
    csv_assembler(const std::vector<std::vector<std::string>> &content, const std::string &path);

  public:
    explicit Abstract_aMLLibrary_Weight_Importer(network_butcher::parameters::Parameters const &params)
      : Weight_Importer()
      , block_graph_generation_params{params.block_graph_generation_params}
      , aMLLibrary_params{params.aMLLibrary_params}
      , weights_params{params.weights_params}
      , model_params{params.model_params}
      , devices{params.devices}
    {
      check_aMLLibrary();
    };

    explicit Abstract_aMLLibrary_Weight_Importer(
      parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params,
      parameters::Parameters::aMLLibrary const             &aMLLibrary_params,
      parameters::Parameters::Weights const                &weights_params,
      parameters::Parameters::Model const                  &model_params,
      parameters::Parameters::Devices const                &devices)
      : Weight_Importer()
      , block_graph_generation_params{block_graph_generation_params}
      , aMLLibrary_params{aMLLibrary_params}
      , weights_params{weights_params}
      , model_params{model_params}
      , devices{devices}
    {
      check_aMLLibrary();
    };

    ~Abstract_aMLLibrary_Weight_Importer() override = default;
  };
} // namespace network_butcher::io


#endif // NETWORK_BUTCHER_BASIC_AMLLIBRARY_WEIGHT_IMPOTER_H
