//
// Created by faccus on 13/04/23.
//
#include "basic_aMLLibrary_Weight_Importer.h"

namespace network_butcher::io
{
  void
  basic_aMLLibrary_Weight_Importer::check_aMLLibrary() const
  {
#if PYBIND_ACTIVE
    if (params.backward_connections_allowed)
      throw std::logic_error("aMLLibrary only supports graphs with no backward connections");
#else
    throw std::logic_error("aMLLibrary not supported. Please compile with PYBIND_ACTIVE"); //
#endif
  }


  void
  basic_aMLLibrary_Weight_Importer::add_python_packages() const
  {
#if PYBIND_ACTIVE
    using namespace pybind11::literals;
    namespace py = pybind11;

    py::object path     = py::module_::import("sys").attr("path");
    py::object inserter = path.attr("append");

    std::string const dep_import = Utilities::combine_path(std::string(NN_Source_Path), "dep");

    inserter(dep_import);

#  if PLATFORM_SPECIFIC_CONFIG
    std::string const local_lib_path = std::string(PYTHON_LOCAL_LIB_PATH);

    inserter(local_lib_path);
#  endif

    for (auto const &package_location : params.aMLLibrary_params.extra_packages_location)
      inserter(package_location);
#endif
  }


  void
  basic_aMLLibrary_Weight_Importer::csv_assembler(std::vector<std::vector<std::string>> const &content,
                                                  std::string const                           &path) const
  {
    std::fstream file_out;
    file_out.open(path, std::ios_base::out);

    for (auto const &row : content)
      {
        for (std::size_t i = 0; i < row.size(); ++i)
          {
            file_out << row[i];
            if (i != row.size() - 1)
              file_out << ",";
          }
        file_out << std::endl;
      }

    file_out.close();
  }


  void
  basic_aMLLibrary_Weight_Importer::execute_weight_generator(const std::string &regressor_file,
                                                             const std::string &config_file,
                                                             const std::string &output_path) const
  {
#if PYBIND_ACTIVE
    using namespace pybind11::literals;
    namespace py = pybind11;

    py::object Predictor = py::module_::import("aMLLibrary.model_building.predictor").attr("Predictor");
    py::object predict =
      Predictor("regressor_file"_a = regressor_file, "output_folder"_a = output_path, "debug"_a = false)
        .attr("predict");

    predict("config_file"_a = config_file, "mape_to_file"_a = false);
#endif
  }


  std::string
  basic_aMLLibrary_Weight_Importer::network_info_onnx_tool() const
  {
#if PYBIND_ACTIVE
    using namespace pybind11::literals;
    namespace py = pybind11;

    auto const &aMLLibrary_params = params.aMLLibrary_params;

    if (!Utilities::directory_exists(aMLLibrary_params.temporary_directory))
      Utilities::create_directory(aMLLibrary_params.temporary_directory);

    auto weight_path = Utilities::combine_path(aMLLibrary_params.temporary_directory, "weights.csv");
    if (Utilities::file_exists(weight_path))
      Utilities::file_delete(weight_path);

    py::object onnx_tool = py::module_::import("onnx_tool");

    py::object model_profile = onnx_tool.attr("model_profile");
    model_profile(params.model_path, "savenode"_a = weight_path);

    return weight_path;
#else
    throw;                                                                                 // ("aMLLibrary not supported. Please compile with PYBIND_ACTIVE");
#endif
  }


  std::map<std::string, Weight_importer_helpers::onnx_tool_output>
  basic_aMLLibrary_Weight_Importer::read_network_info_onnx_tool(const std::string &path) const
  {
    using namespace Weight_importer_helpers;
    std::map<std::string, onnx_tool_output> res;

    auto data = read_csv(path, ',', {"name", "macs", "memory", "params"});

    auto const &names = data["name"];
    auto const &macs  = data["macs"];
    auto const &mem   = data["memory"];
    auto const &param = data["params"];


    for (std::size_t i = 0; i < names.size(); ++i)
      {
        onnx_tool_output info;

        info.name   = Utilities::to_lowercase_copy(names[i]);
        info.macs   = std::stoul(macs[i]);
        info.memory = std::stoul(mem[i]);
        info.params = std::stoul(param[i]);

        res.emplace(info.name, std::move(info));
      }

    return res;
  }


  void
  basic_aMLLibrary_Weight_Importer::prepare_predict_file(std::string const &inference_variable,
                                                         std::string const &input_path,
                                                         std::string        output_path) const
  {
    if (output_path.back() == '/' || output_path.back() == '\\')
      {
        output_path += "predict.ini";
      }

    std::ofstream out_file(output_path);

    if (out_file.is_open())
      {
        out_file << "[General]" << std::endl;
        out_file << "y = " << inference_variable << std::endl << std::endl;

        out_file << "[DataPreparation]" << std::endl;
        out_file << "input_path = " << input_path;

        out_file.close();
      }
  }

} // namespace network_butcher::io