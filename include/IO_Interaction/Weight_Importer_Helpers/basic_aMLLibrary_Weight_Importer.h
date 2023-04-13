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

#include "Computer_memory.h"
#include "Weight_Importer.h"
#include "Weight_Importer_Utils.h"

namespace network_butcher::io
{
  class basic_aMLLibrary_Weight_Importer : public Weight_Importer
  {
  protected:
    network_butcher::parameters::Parameters const params;


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
    void
    prepare_predict_file(std::string const &inference_variable,
                         std::string const &input_path,
                         std::string        output_path = "") const;


    /// It will execute the weight generator
    /// \param regressor_file The regressor file
    /// \param config_file The configuration file
    /// \param output_path The output path
    void
    execute_weight_generator(const std::string &regressor_file,
                             const std::string &config_file,
                             const std::string &output_path) const;


    std::string
    network_info_onnx_tool() const;


    std::map<std::string, Weight_importer_helpers::onnx_tool_output>
    read_network_info_onnx_tool(const std::string &path) const;


    void
    csv_assembler(const std::vector<std::vector<std::string>> &content, const std::string &path) const;

  public:
    basic_aMLLibrary_Weight_Importer(network_butcher::parameters::Parameters const &params)
      : Weight_Importer()
      , params{params}
    {
      check_aMLLibrary();
    };

    virtual ~basic_aMLLibrary_Weight_Importer() = default;
  };
} // namespace network_butcher::io


#endif // NETWORK_BUTCHER_BASIC_AMLLIBRARY_WEIGHT_IMPOTER_H
