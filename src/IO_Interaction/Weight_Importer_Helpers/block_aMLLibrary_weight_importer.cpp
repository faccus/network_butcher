#include <network_butcher/IO_Interaction/Weight_Importer_Helpers/block_aMLLibrary_weight_importer.h>

namespace network_butcher::io
{
  void
  block_aMLLibrary_Weight_Importer::check_aMLLibrary() const
  {
#if NETWORK_BUTCHER_PYBIND_ACTIVE
    if (weights_params.bandwidth->size() != 2)
      throw std::logic_error("block_aMLLibrary_Weight_Importer: aMLLibrary only supports graphs with two devices");

    if (weights_params.bandwidth->check_weight(std::make_pair(1, 0)))
      throw std::logic_error("block_aMLLibrary_Weight_Importer: aMLLibrary doesn't support backward connections");
#else
    throw std::logic_error("block_aMLLibrary_Weight_Importer: aMLLibrary not supported. Please compile with "
                           "NETWORK_BUTCHER_PYBIND_ACTIVE"); //
#endif
  }


  void
  block_aMLLibrary_Weight_Importer::add_python_packages() const
  {
#if NETWORK_BUTCHER_PYBIND_ACTIVE
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

    for (auto const &package_location : aMLLibrary_params.extra_packages_location)
      inserter(package_location);
#else
    throw std::logic_error("block_aMLLibrary_Weight_Importer: aMLLibrary not supported. Please compile with "
                           "NETWORK_BUTCHER_PYBIND_ACTIVE"); //
#endif
  }


  void
  block_aMLLibrary_Weight_Importer::csv_assembler(std::vector<std::vector<std::string>> const &content,
                                                  std::string const                           &path)
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
  block_aMLLibrary_Weight_Importer::execute_weight_generator(const std::string &regressor_file,
                                                             const std::string &config_file,
                                                             const std::string &output_path) const
  {
#if NETWORK_BUTCHER_PYBIND_ACTIVE
    using namespace pybind11::literals;
    namespace py = pybind11;

    py::object Predictor = py::module_::import("aMLLibrary.model_building.predictor").attr("Predictor");
    py::object predict =
      Predictor("regressor_file"_a = regressor_file, "output_folder"_a = output_path, "debug"_a = false)
        .attr("predict");

    predict("config_file"_a = config_file, "mape_to_file"_a = false);
#else
    throw std::logic_error("block_aMLLibrary_Weight_Importer: aMLLibrary not supported. Please compile with "
                           "NETWORK_BUTCHER_PYBIND_ACTIVE"); //
#endif
  }


  auto
  block_aMLLibrary_Weight_Importer::network_info_onnx_tool() const -> std::string
  {
#if NETWORK_BUTCHER_PYBIND_ACTIVE
    using namespace pybind11::literals;
    namespace py = pybind11;

    if (!Utilities::directory_exists(aMLLibrary_params.temporary_directory))
      Utilities::create_directory(aMLLibrary_params.temporary_directory);

    auto weight_path = Utilities::combine_path(aMLLibrary_params.temporary_directory, "weights.csv");
    if (Utilities::file_exists(weight_path))
      Utilities::file_delete(weight_path);

    py::object onnx_tool = py::module_::import("onnx_tool");

    py::object model_profile = onnx_tool.attr("model_profile");
    model_profile(model_params.model_path, "savenode"_a = weight_path);

    return weight_path;
#else
    throw std::logic_error(
      "block_aMLLibrary_Weight_Importer: aMLLibrary not supported. Please compile with NETWORK_BUTCHER_PYBIND_ACTIVE");
#endif
  }


  auto
  block_aMLLibrary_Weight_Importer::read_network_info_onnx_tool(const std::string &path)
    -> std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type>
  {
    using namespace Weight_importer_helpers;
    std::map<std::string, Onnx_Tool_Output_Type> res;

    auto data = read_csv(path, ',', {"name", "macs", "memory", "params"});

    auto const &names = data["name"];
    auto const &macs  = data["macs"];
    auto const &mem   = data["memory"];
    auto const &param = data["params"];


    for (std::size_t i = 0; i < names.size(); ++i)
      {
        Onnx_Tool_Output_Type info;

        info.name   = Utilities::to_lowercase_copy(names[i]);
        info.macs   = std::stoul(macs[i]);
        info.memory = std::stoul(mem[i]);
        info.params = std::stoul(param[i]);

        res.emplace(info.name, std::move(info));
      }

    return res;
  }


  void
  block_aMLLibrary_Weight_Importer::prepare_predict_file(std::string const &inference_variable,
                                                         std::string const &input_path,
                                                         std::string        output_path)
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


  auto
  block_aMLLibrary_Weight_Importer::generate_entry(
    std::string const                                                           &lower_case,
    std::size_t                                                                  id,
    std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type> const &map_onnx_tool,
    std::map<std::string, std::size_t>                                          &previous_entries_info,
    Node_Id_Collection_Type const                                               &original_ids,
    Node_Id_Collection_Type const                                               &node_output_ids) const -> std::string
  {
    if (lower_case == "layer")
      {
        return original_graph[*graph[id].content.second->crbegin()].name;
      }
    else if (lower_case == "tensorlength")
      {
        {
          auto const it = previous_entries_info.find("tensorlength");
          if (it != previous_entries_info.cend())
            {
              return std::to_string(it->second);
            }
        }

        std::size_t tensor_length = 0;
        if (original_ids.size() == 1)
          {
            auto const &original_node = original_graph[*original_ids.cbegin()];

            for (auto const &out : original_node.content.get_output())
              tensor_length += out.second->compute_shape_volume();
          }
        else
          {
            auto const &original_node = original_graph[*node_output_ids.cbegin()];

            for (auto const &out : original_node.content.get_input())
              tensor_length += out.second->compute_shape_volume();
          }

        previous_entries_info["tensorlength"] = tensor_length;
        return std::to_string(tensor_length);
      }
    else if (lower_case == "networkingtime")
      {
        {
          auto const it = previous_entries_info.find("networkingtime");
          if (it != previous_entries_info.cend())
            {
              return std::to_string(it->second);
            }
        }
        std::size_t mem = 0;

        {
          auto const it = previous_entries_info.find("memory");
          if (it != previous_entries_info.cend())
            {
              mem = it->second;
            }
          else
            {
              for (auto const &node_id : original_ids)
                mem += network_butcher::computer::Computer_memory::compute_memory_usage_output(original_graph[node_id]);
              previous_entries_info["memory"] = mem;
            }
        }

        auto const &[band, access] = weights_params.bandwidth->get_weight(std::make_pair(0, 1));
        auto const net_time        = access + mem * static_cast<long double>(8) / (band * std::pow(10, 6));
        return std::to_string(net_time);
      }
    else if (lower_case == "optype")
      {
        return original_graph[*original_ids.cbegin()].name;
      }
    else if (lower_case == "nrparameters")
      {
        {
          auto const it = previous_entries_info.find("nrparameters");
          if (it != previous_entries_info.cend())
            {
              return std::to_string(it->second);
            }
        }

        std::size_t nr_param = 0;
        for (auto const &original_id : original_ids)
          {
            for (auto const &parameter : original_graph[original_id].content.get_parameters())
              nr_param += parameter.second->compute_shape_volume();
          }

        previous_entries_info["nrparameters"] = nr_param;
        return std::to_string(nr_param);
      }
    else if (lower_case == "memory")
      {
        {
          auto const it = previous_entries_info.find("memory");
          if (it != previous_entries_info.cend())
            {
              return std::to_string(it->second);
            }
        }
        std::size_t mem = 0;
        for (auto const &node_id : original_ids)
          mem += network_butcher::computer::Computer_memory::compute_memory_usage_output(original_graph[node_id]);

        previous_entries_info["memory"] = mem;
        return std::to_string(mem);
      }
    else if (lower_case == "macs")
      {
        {
          auto const it = previous_entries_info.find("macs");
          if (it != previous_entries_info.cend())
            {
              return std::to_string(it->second);
            }
        }

        std::size_t macs = 0;
        for (auto const &original_id : original_ids)
          {
            auto const it = map_onnx_tool.find(original_graph[original_id].name);
            if (it != map_onnx_tool.cend())
              macs += it->second.macs;
          }

        previous_entries_info["macs"] = macs;
        return std::to_string(macs);
      }
    else if (lower_case == "nrnodes")
      {
        return std::to_string(original_ids.size());
      }
    else
      {
        return "";
      }
  }


  auto
  block_aMLLibrary_Weight_Importer::generate_entries(
    const std::vector<std::string>                                              &entries,
    std::size_t                                                                  id,
    const std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type> &map_onnx_tool) const
    -> std::vector<std::string>
  {
    auto const lower_case_entries = Utilities::to_lowercase_copy(entries);

    std::map<std::string, std::size_t> inserted;
    std::vector<std::string>           res;
    res.reserve(entries.size());

    auto const &original_ids    = *graph[id].content.second;
    auto const &node_output_ids = *graph[*graph.get_output_nodes(id).cbegin()].content.second;

    for (auto const &lower_case : lower_case_entries)
      {
        res.emplace_back(generate_entry(lower_case, id, map_onnx_tool, inserted, original_ids, node_output_ids));
      }

    return res;
  }


  auto
  block_aMLLibrary_Weight_Importer::perform_predictions(std::string const &csv_path) const
    -> std::pair<std::vector<std::string>, std::vector<std::string>>
  {
    std::vector<std::string> paths;
    std::vector<std::string> relevant_entries;

    for (std::size_t i = 0; i < devices.size(); ++i)
      {
        std::string tmp_dir_path =
          Utilities::combine_path(aMLLibrary_params.temporary_directory, "predict_" + std::to_string(i));

        Utilities::directory_delete(tmp_dir_path);

        prepare_predict_file(aMLLibrary_params.aMLLibrary_inference_variables[i], csv_path, tmp_dir_path + ".ini");

        execute_weight_generator(devices[i].weights_path, tmp_dir_path + ".ini", tmp_dir_path);

        paths.emplace_back(Utilities::combine_path(tmp_dir_path, "prediction.csv"));
        relevant_entries.emplace_back("pred");
      }

    return std::make_pair(paths, relevant_entries);
  }


  void
  block_aMLLibrary_Weight_Importer::import_weights()
  {
#if NETWORK_BUTCHER_PYBIND_ACTIVE
    pybind11::initialize_interpreter();

    add_python_packages();

    auto const macs     = read_network_info_onnx_tool(network_info_onnx_tool());
    auto const csv_path = "aMLLibrary_input.csv";

    Utilities::create_directory(aMLLibrary_params.temporary_directory);
    Utilities::file_delete(csv_path);

    // Prepare the .csv file to be fed to aMLLibrary
    std::vector<std::vector<std::string>> aMLLibrary_input;
    aMLLibrary_input.reserve(graph.size() + 1);

    aMLLibrary_input.push_back(Utilities::trim_copy(aMLLibrary_params.aMLLibrary_csv_features));
    aMLLibrary_input.front().insert(aMLLibrary_input.front().cend(),
                                    aMLLibrary_params.aMLLibrary_inference_variables.cbegin(),
                                    aMLLibrary_params.aMLLibrary_inference_variables.cend());

    // Generate entries for the .csv file
    for (auto const &node : graph.get_nodes())
      {
        if (node.content.first != 0 || !node.content.second->empty() &&
                                         (original_graph[*node.content.second->cbegin()].name == "__fake__input__" ||
                                          original_graph[*node.content.second->cbegin()].name == "__fake__output__"))
          continue;

        aMLLibrary_input.push_back(generate_entries(aMLLibrary_input.front(), node.get_id(), macs));
      }

    // Assemble the .csv file
    csv_assembler(aMLLibrary_input, csv_path);

    // Construct the weights through aMMLibrary
    auto [paths, relevant_entries] = perform_predictions(csv_path);

    pybind11::finalize_interpreter();

    Csv_Weight_Importer importer(graph, paths, relevant_entries, devices, weights_params.separator, true);
    importer.import_weights();
#else
    throw std::logic_error("block_aMLLibrary_Weight_Importer: aMLLibrary not supported. Please compile with "
                           "NETWORK_BUTCHER_PYBIND_ACTIVE"); //
#endif
  }
} // namespace network_butcher::io