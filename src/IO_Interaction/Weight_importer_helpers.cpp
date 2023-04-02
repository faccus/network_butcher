#include "Weight_importer_helpers.h"

namespace network_butcher::io::Weight_importer_helpers
{
  csv_result_type<double>
  read_csv_numerics(std::string const &path, char separator, std::vector<std::string> columns_to_read)
  {
    std::ifstream file(path);

    csv_result_type<double> res;


    if (file.is_open())
      {
        Utilities::to_lowercase(columns_to_read);
        Utilities::trim(columns_to_read);

        std::vector<std::size_t>           indices;
        std::string                        tmp_line;
        std::map<std::size_t, std::string> index_map;

        if (!std::getline(file, tmp_line))
          return res;

        {
          std::stringstream reader(tmp_line);
          std::size_t       i = 0;

          if (!columns_to_read.empty())
            {
              while (std::getline(reader, tmp_line, separator))
                {
                  Utilities::to_lowercase(tmp_line);
                  Utilities::trim(tmp_line);

                  if (std::find(columns_to_read.cbegin(), columns_to_read.cend(), tmp_line) != columns_to_read.cend())
                    {
                      indices.push_back(i);
                      index_map[i] = tmp_line;
                    }

                  ++i;
                }
            }
          else
            {
              while (std::getline(reader, tmp_line, separator))
                {
                  Utilities::to_lowercase(tmp_line);
                  Utilities::trim(tmp_line);

                  indices.push_back(i);
                  index_map[i] = tmp_line;

                  ++i;
                }
            }
        }


        while (std::getline(file, tmp_line))
          {
            std::stringstream reader(tmp_line);
            std::size_t       i = 0, j = 0;

            while (std::getline(reader, tmp_line, separator))
              {
                if (indices[j] == i)
                  {
                    res[index_map[indices[j]]].push_back(std::atof(tmp_line.c_str()));
                    ++j;
                  }

                ++i;
              }
          }
      }

    return res;
  }

  csv_result_type<std::string>
  read_csv(std::string const &path, char separator, std::vector<std::string> columns_to_read)
  {
    std::ifstream file(path);

    csv_result_type<std::string> res;


    if (file.is_open())
      {
        Utilities::to_lowercase(columns_to_read);
        Utilities::trim(columns_to_read);

        std::vector<std::size_t>           indices;
        std::string                        tmp_line;
        std::map<std::size_t, std::string> index_map;

        if (!std::getline(file, tmp_line))
          return res;

        {
          std::stringstream reader(tmp_line);
          std::size_t       i = 0;

          if (!columns_to_read.empty())
            {
              while (std::getline(reader, tmp_line, separator))
                {
                  Utilities::to_lowercase(tmp_line);
                  Utilities::trim(tmp_line);

                  if (std::find(columns_to_read.cbegin(), columns_to_read.cend(), tmp_line) != columns_to_read.cend())
                    {
                      indices.push_back(i);
                      index_map[i] = tmp_line;
                    }

                  ++i;
                }
            }
          else
            {
              while (std::getline(reader, tmp_line, separator))
                {
                  Utilities::to_lowercase(tmp_line);
                  Utilities::trim(tmp_line);

                  indices.push_back(i);
                  index_map[i] = tmp_line;

                  ++i;
                }
            }
        }


        while (std::getline(file, tmp_line))
          {
            std::stringstream reader(tmp_line);
            std::size_t       i = 0, j = 0;

            while (std::getline(reader, tmp_line, separator))
              {
                if (indices[j] == i)
                  {
                    res[index_map[indices[j]]].push_back(tmp_line.c_str());
                    ++j;
                  }

                ++i;
              }
          }
      }

    return res;
  }
} // namespace network_butcher::io::Weight_importer_helpers


namespace network_butcher::io
{
  void
  Csv_Weight_Importer::import_weights(std::function<bool(graph_type::Node_Type const &)> const &extra_condition)
  {
    if (paths.empty() || (paths.size() != 1 && paths.size() != relevant_entries.size()) ||
        paths.size() > devices.size() || relevant_entries.size() != devices.size())
      throw;

    Weight_importer_helpers::csv_result_type<double> data;

    if (paths.size() == 1)
      {
        data = Weight_importer_helpers::read_csv_numerics(paths[0], separator, relevant_entries);
      }
    else
      {
        for (std::size_t i = 0; i < paths.size(); ++i)
          {
            auto tmp = Weight_importer_helpers::read_csv_numerics(paths[i], separator, {relevant_entries[i]});
            data.insert(tmp.begin(), tmp.end());
          }
      }

    for (std::size_t i = 0; i < devices.size(); ++i)
      {
        auto const &device = devices[i];
        auto const &entry  = relevant_entries[i];

        auto const data_it = data.find(entry);

        if (data_it == data.cend())
          throw; // ("Missing entry for device Id " + std::to_string(i));

        auto const &values = data_it->second;
        auto        it     = ++graph.cbegin();

        for (auto const &weight : values)
          {
            while (extra_condition != nullptr && it != graph.cend() && !extra_condition(*it))
              ++it;

            if (it == graph.cend())
              break;

            for (auto const &in : graph.get_neighbors()[it->get_id()].first)
              {
                graph.set_weight(devices[i], {in, it->get_id()}, weight);
              }

            ++it;
          }
      }
  }


  void
  Csv_Weight_Importer::import_weights()
  {
    import_weights(nullptr);
  }
} // namespace network_butcher::io


namespace network_butcher::io
{
  void
  basic_aMLLibrary_Weight_Importer::check_aMLLibrary() const
  {
#if PYBIND_ACTIVE
    if (params.devices.size() != 2 || params.backward_connections_allowed)
      throw; // ("aMLLibrary only supports 2 devices and no backward connections");
#else
    throw; // ("aMLLibrary not supported. Please compile with PYBIND_ACTIVE");
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
  basic_aMLLibrary_Weight_Importer::add_python_packages(std::vector<std::string> const &extra_packages_location) const
  {
#if PYBIND_ACTIVE
    using namespace pybind11::literals;
    namespace py = pybind11;

    py::object path     = py::module_::import("sys").attr("path");
    py::object inserter = path.attr("append");

    inserter(NN_Source_Path);

    for (auto const &package_location : extra_packages_location)
      inserter(package_location);
#endif
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
  basic_aMLLibrary_Weight_Importer::network_info_onnx_tool(const network_butcher::parameters::Parameters &params) const
  {
#if PYBIND_ACTIVE
    using namespace pybind11::literals;
    namespace py = pybind11;

    if (!Utilities::directory_exists(params.temporary_directory))
      Utilities::create_directory(params.temporary_directory);

    auto weight_path = Utilities::combine_path(params.temporary_directory, "weights.csv");
    if (Utilities::file_exists(weight_path))
      Utilities::file_delete(weight_path);

    py::object onnx_tool = py::module_::import("onnx_tool");

    py::object model_profile = onnx_tool.attr("model_profile");
    model_profile(params.model_path, "savenode"_a = weight_path);

    return weight_path;
#else
    throw; // ("aMLLibrary not supported. Please compile with PYBIND_ACTIVE");
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


namespace network_butcher::io
{
  std::string
  original_aMLLibrary_Weight_Importer::generate_entry(const std::string                             &entry,
                                                      const graph_type::Node_Type                   &node,
                                                      const network_butcher::parameters::Parameters &params) const
  {
    auto const lower_case = Utilities::to_lowercase_copy(entry);
    if (lower_case == "nrparameters")
      {
        std::size_t cnt = 0;
        for (auto const &par : node.content.get_parameters())
          cnt += par.second->compute_shape_volume();
        return std::to_string(cnt);
      }
    else if (lower_case == "memory")
      return std::to_string(network_butcher::computer::Computer_memory::compute_memory_usage(node));
    else if (lower_case == "macs")
      return "0";
    else
      return generate_entry(entry, {}, node, params);
  }


  std::string
  original_aMLLibrary_Weight_Importer::generate_entry(const std::string                               &entry,
                                                      const Weight_importer_helpers::onnx_tool_output &basic_info,
                                                      const graph_type::Node_Type                     &node,
                                                      const network_butcher::parameters::Parameters   &params) const
  {
    // std::vector<std::string> header{"Layer", "TensorLength", "OpType", "NrParameters", "Memory", "MACs"};
    auto const lower_case = Utilities::to_lowercase_copy(entry);

    if (lower_case == "layer")
      return node.name;
    else if (lower_case == "tensorlength")
      {
        std::size_t tensor_length = 0;
        for (auto const &out : node.content.get_output())
          tensor_length += out.second->compute_shape_volume();
        return std::to_string(tensor_length);
      }
    else if (lower_case == "networkingtime")
      {
        auto const net_time = params.bandwidth.cbegin()->second.second +
                              network_butcher::computer::Computer_memory::compute_memory_usage_output(node) * 8 /
                                (params.bandwidth.cbegin()->second.first * std::pow(10, 6));
        return std::to_string(net_time);
      }
    else if (lower_case == "optype")
      return node.content.get_operation_id();
    else if (lower_case == "nrparameters")
      return std::to_string(basic_info.params);
    else if (lower_case == "memory")
      return std::to_string(basic_info.memory);
    else if (lower_case == "macs")
      return std::to_string(basic_info.macs);
    else if (lower_case == "nrnodes")
      return "1";
    else
      return "0";
  }


  void
  original_aMLLibrary_Weight_Importer::import_weights()
  {
    import_weights(nullptr);
  }


  void
  original_aMLLibrary_Weight_Importer::import_weights(
    std::function<bool(graph_type::Node_Type const &)> const &extra_condition)
  {
    pybind11::initialize_interpreter();
    add_python_packages(params.extra_packages_location);

    auto const macs     = read_network_info_onnx_tool(network_info_onnx_tool(params));
    auto const csv_path = "aMLLibrary_input.csv";

    if (!Utilities::directory_exists(params.temporary_directory))
      Utilities::create_directory(params.temporary_directory);

    if (Utilities::file_exists(csv_path))
      Utilities::file_delete(csv_path);

    std::vector<std::vector<std::string>> aMLLibrary_input;
    aMLLibrary_input.reserve(graph.size() + 1);

    aMLLibrary_input.push_back(Utilities::trim_copy(params.aMLLibrary_csv_features));
    aMLLibrary_input.front().insert(aMLLibrary_input.front().cend(),
                                    params.aMLLibrary_inference_variables.cbegin(),
                                    params.aMLLibrary_inference_variables.cend());

    for (auto const &node : graph.get_nodes())
      {
        auto                     info_it = macs.find(node.name);
        std::vector<std::string> row;
        row.reserve(aMLLibrary_input.front().size());

        if (info_it != macs.cend())
          {
            for (auto const &entry : aMLLibrary_input.front())
              {
                row.emplace_back(generate_entry(entry, info_it->second, node, params));
              }
          }
        else
          {
            for (auto const &entry : aMLLibrary_input.front())
              {
                row.emplace_back(generate_entry(entry, node, params));
              }
          }

        aMLLibrary_input.push_back(std::move(row));
      }

    csv_assembler(aMLLibrary_input, csv_path);

    std::vector<std::string> paths;
    std::vector<std::string> relevant_entries;

    for (std::size_t i = 0; i < params.devices.size(); ++i)
      {
        std::string tmp_dir_path = Utilities::combine_path(params.temporary_directory, "predict_" + std::to_string(i));

        if (Utilities::directory_exists(tmp_dir_path))
          Utilities::directory_delete(tmp_dir_path);

        prepare_predict_file(params.aMLLibrary_inference_variables[i], csv_path, tmp_dir_path + ".ini");

        execute_weight_generator(params.devices[i].weights_path, tmp_dir_path + ".ini", tmp_dir_path);

        paths.emplace_back(Utilities::combine_path(tmp_dir_path, "prediction.csv"));
        relevant_entries.emplace_back("pred");
      }

    pybind11::finalize_interpreter();

    Csv_Weight_Importer importer(graph, paths, relevant_entries, params.devices, params.separator);
    importer.import_weights(extra_condition);
  }
} // namespace network_butcher::io


namespace network_butcher::io
{
  std::vector<std::string>
  block_aMLLibrary_Weight_Importer::generate_entry(
    std::vector<std::string> const                                         &entries,
    network_butcher::parameters::Parameters const                          &params,
    block_graph_type const                                                 &new_graph,
    graph_type const                                                       &graph,
    std::size_t                                                             id,
    std::map<std::string, Weight_importer_helpers::onnx_tool_output> const &map_onnx_tool) const
  {
    auto const v_lower_case = Utilities::to_lowercase_copy(entries);

    std::map<std::string, std::size_t> inserted;
    std::vector<std::string>           res;
    res.reserve(entries.size());

    auto const &original_ids    = *new_graph[id].content.second;
    auto const &node_output_ids = new_graph[*new_graph.get_neighbors()[id].second.cbegin()].content.second;

    for (auto const &lower_case : v_lower_case)
      {
        if (lower_case == "layer")
          {
            res.push_back(graph[*new_graph[id].content.second->crbegin()].name);
          }
        else if (lower_case == "tensorlength")
          {
            {
              auto const it = inserted.find("tensorlength");
              if (it != inserted.cend())
                {
                  res.push_back(std::to_string(it->second));
                  continue;
                }
            }

            std::size_t tensor_length = 0;
            if (original_ids.size() == 1)
              {
                auto const &original_node = graph[*original_ids.cbegin()];

                for (auto const &out : original_node.content.get_output())
                  tensor_length += out.second->compute_shape_volume();
              }
            else
              {
                auto const &original_node = graph[*node_output_ids->cbegin()];

                for (auto const &out : original_node.content.get_input())
                  tensor_length += out.second->compute_shape_volume();
              }

            inserted["tensorlength"] = tensor_length;
            res.push_back(std::to_string(tensor_length));
          }
        else if (lower_case == "networkingtime")
          {
            {
              auto const it = inserted.find("networkingtime");
              if (it != inserted.cend())
                {
                  res.push_back(std::to_string(it->second));
                  continue;
                }
            }
            std::size_t mem = 0;

            {
              auto const it = inserted.find("memory");
              if (it != inserted.cend())
                {
                  mem = it->second;
                }
              else
                {
                  for (auto const &node_id : original_ids)
                    mem += network_butcher::computer::Computer_memory::compute_memory_usage_output(graph[node_id]);
                  inserted["memory"] = mem;
                }
            }

            auto const net_time = params.bandwidth.cbegin()->second.second +
                                  mem * 8 / (params.bandwidth.cbegin()->second.first * std::pow(10, 6));
            res.push_back(std::to_string(net_time));
          }
        else if (lower_case == "optype")
          {
            res.push_back(graph[*original_ids.cbegin()].name);
          }
        else if (lower_case == "nrparameters")
          {
            {
              auto const it = inserted.find("nrparameters");
              if (it != inserted.cend())
                {
                  res.push_back(std::to_string(it->second));
                  continue;
                }
            }

            std::size_t nr_param = 0;
            for (auto const &original_id : original_ids)
              {
                for (auto const &parameter : graph[original_id].content.get_parameters())
                  nr_param += parameter.second->compute_shape_volume();
              }

            inserted["nrparameters"] = nr_param;
            res.push_back(std::to_string(nr_param));
          }
        else if (lower_case == "memory")
          {
            {
              auto const it = inserted.find("memory");
              if (it != inserted.cend())
                {
                  res.push_back(std::to_string(it->second));
                  continue;
                }
            }
            std::size_t mem = 0;
            for (auto const &node_id : original_ids)
              mem += network_butcher::computer::Computer_memory::compute_memory_usage_output(graph[node_id]);

            inserted["memory"] = mem;
            res.push_back(std::to_string(mem));
          }
        else if (lower_case == "macs")
          {
            {
              auto const it = inserted.find("macs");
              if (it != inserted.cend())
                {
                  res.push_back(std::to_string(it->second));
                  continue;
                }
            }

            std::size_t macs = 0;
            for (auto const &original_id : original_ids)
              {
                auto const it = map_onnx_tool.find(graph[original_id].name);
                if (it != map_onnx_tool.cend())
                  macs += it->second.macs;
              }

            inserted["macs"] = macs;
            res.push_back(std::to_string(macs));
          }
        else if (lower_case == "nrnodes")
          {
            res.push_back(std::to_string(original_ids.size()));
          }
        else
          res.push_back("");
      }

    return res;
  }


  void
  block_aMLLibrary_Weight_Importer::import(
    std::size_t                                                     device,
    std::string const                                              &path,
    std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition = nullptr)
  {
    auto const map = Weight_importer_helpers::read_csv_numerics(path, ',', {"pred"});

    auto it = new_graph.cbegin();

    for (std::size_t j = 0; it != new_graph.cend() && j < (device + 1); ++j, ++it)
      ;

    for (auto const &weight : map.cbegin()->second)
      {
        while (extra_condition != nullptr && it != new_graph.cend() && !extra_condition(*it))
          {
            for (std::size_t j = 0; it != new_graph.cend() && j < params.devices.size(); ++j, ++it)
              ;
          }

        if (it == new_graph.cend())
          break;

        for (auto const &in : new_graph.get_neighbors()[it->get_id()].first)
          new_graph.set_weight({in, it->get_id()}, weight);

        for (std::size_t j = 0; it != new_graph.cend() && j < params.devices.size(); ++j, ++it)
          ;
      }
  }

  void
  block_aMLLibrary_Weight_Importer::import_weights()
  {
    import_weights(nullptr);
  }

  void
  block_aMLLibrary_Weight_Importer::import_weights(
    std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition)
  {
    pybind11::initialize_interpreter();

    add_python_packages(params.extra_packages_location);

    auto const macs     = read_network_info_onnx_tool(network_info_onnx_tool(params));
    auto const csv_path = "aMLLibrary_input.csv";

    if (!Utilities::directory_exists(params.temporary_directory))
      Utilities::create_directory(params.temporary_directory);

    if (Utilities::file_exists(csv_path))
      Utilities::file_delete(csv_path);

    std::vector<std::vector<std::string>> aMLLibrary_input;
    aMLLibrary_input.reserve(new_graph.size() + 1);
    aMLLibrary_input.push_back(Utilities::trim_copy(params.aMLLibrary_csv_features));
    aMLLibrary_input.front().insert(aMLLibrary_input.front().cend(),
                                    params.aMLLibrary_inference_variables.cbegin(),
                                    params.aMLLibrary_inference_variables.cend());

    for (auto const &node : new_graph.get_nodes())
      {
        if (node.content.first != 0 ||
            !node.content.second->empty() && (graph[*node.content.second->cbegin()].name == "__fake__input__" ||
                                              graph[*node.content.second->cbegin()].name == "__fake__output__"))
          continue;

        aMLLibrary_input.push_back(
          generate_entry(aMLLibrary_input.front(), params, new_graph, graph, node.get_id(), macs));
      }


    csv_assembler(aMLLibrary_input, csv_path);

    for (std::size_t i = 0; i < params.devices.size(); ++i)
      {
        std::string tmp_dir_path = Utilities::combine_path(params.temporary_directory, "predict_" + std::to_string(i));

        if (Utilities::directory_exists(tmp_dir_path))
          Utilities::directory_delete(tmp_dir_path);

        prepare_predict_file(params.aMLLibrary_inference_variables[i], csv_path, tmp_dir_path + ".ini");

        execute_weight_generator(params.devices[i].weights_path, tmp_dir_path + ".ini", tmp_dir_path);

        import(params.devices[i].id, Utilities::combine_path(tmp_dir_path, "prediction.csv"), extra_condition);
      }

    pybind11::finalize_interpreter();
  }
} // namespace network_butcher::io