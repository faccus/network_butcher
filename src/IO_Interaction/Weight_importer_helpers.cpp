#include "Weight_importer_helpers.h"

namespace network_butcher_io::Weight_importer_helpers
{
  void
  import_weights_aMLLibrary_direct_read(graph_type                                               &graph,
                                        std::size_t                                               device,
                                        std::string const                                        &path,
                                        std::function<bool(graph_type::Node_Type const &)> const &extra_condition)
  {
    std::fstream file_in;
    file_in.open(path, std::ios_base::in);

    if (file_in.is_open())
      {
        weight_type tmp_weight;

        std::string tmp_line;
        std::getline(file_in, tmp_line);

        auto it = ++graph.cbegin();
        while (!file_in.eof())
          {
            std::getline(file_in, tmp_line);
            std::stringstream stream_line(tmp_line);

            while (std::getline(stream_line, tmp_line, ','))
              ;

            tmp_weight = ::atof(tmp_line.c_str());

            while (extra_condition != nullptr && it != graph.cend() && !extra_condition(*it))
              ++it;

            if (it == graph.cend())
              break;

            for (auto const &in : graph.get_neighbors()[it->get_id()].first)
              graph.set_weight(device, {in, it->get_id()}, tmp_weight);

            ++it;
          }

        file_in.close();
      }
  }


  void
  import_weights_aMLLibrary_direct_read(block_graph_type                                               &graph,
                                        std::size_t                                                     device,
                                        std::size_t                                                     num_devices,
                                        std::string const                                              &path,
                                        std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition)
  {
    std::fstream file_in;
    file_in.open(path, std::ios_base::in);

    if (file_in.is_open())
      {
        weight_type tmp_weight;

        std::string tmp_line;
        std::getline(file_in, tmp_line);

        auto it = ++graph.cbegin();

        for (std::size_t j = 0; it != graph.cend() && j < device; ++j, ++it)
          ;

        if (it == graph.cend())
          return;

        while (!file_in.eof())
          {
            std::getline(file_in, tmp_line);
            std::stringstream stream_line(tmp_line);

            while (std::getline(stream_line, tmp_line, ','))
              ;

            tmp_weight = ::atof(tmp_line.c_str());

            while (extra_condition != nullptr && it != graph.cend() && !extra_condition(*it))
              {
                for (std::size_t j = 0; it != graph.cend() && j < num_devices; ++j, ++it)
                  ;
              }

            if (it == graph.cend())
              break;

            for (auto const &in : graph.get_neighbors()[it->get_id()].first)
              graph.set_weight({in, it->get_id()}, tmp_weight);

            for (std::size_t j = 0; it != graph.cend() && j < num_devices; ++j, ++it)
              ;
          }

        file_in.close();
      }
  }


  void
  import_weights_custom_csv_operation_time(graph_type &graph, std::size_t device, const std::string &path)
  {
    std::fstream file_in;
    file_in.open(path, std::ios_base::in);
    weight_type tmp_weight;

    auto it = graph.get_nodes().cbegin();

    std::string tmp_line;
    std::getline(file_in, tmp_line);

    while (!file_in.eof())
      {
        std::getline(file_in, tmp_line);
        std::stringstream stream_line(tmp_line);

        std::getline(stream_line, tmp_line, ',');
        auto const operation_name = tmp_line;

        std::getline(stream_line, tmp_line, ',');
        tmp_weight = ::atof(tmp_line.c_str());


        while (it != graph.get_nodes().cend() &&
               Utilities::trim_copy(Utilities::to_lowercase_copy(operation_name)) !=
                 Utilities::trim_copy(Utilities::to_lowercase_copy(it->content.get_operation_id())))
          {
            ++it;
          }

        if (it == graph.get_nodes().cend())
          return;

        for (auto const &successor : graph.get_neighbors()[it->get_id()].second)
          {
            graph.set_weight(device, {it->get_id(), successor}, tmp_weight);
          }

        ++it;
      }

    file_in.close();
  }


  void
  csv_assembler(const std::vector<std::vector<std::string>> &content, const std::string &path)
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


  std::string
  aMLLibrary_original_generate_csv_entry(const std::string                            &entry,
                                         const graph_type::Node_Type                  &node,
                                         const network_butcher_parameters::Parameters &params)
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
      return std::to_string(network_butcher_computer::Computer_memory::compute_memory_usage(node));
    else if (lower_case == "macs")
      return "0";
    else
      return aMLLibrary_original_generate_csv_entry(entry, {}, node, params);
  }


  std::string
  aMLLibrary_original_generate_csv_entry(const std::string                            &entry,
                                         const onnx_tool_output                       &basic_info,
                                         const graph_type::Node_Type                  &node,
                                         const network_butcher_parameters::Parameters &params)
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
                              network_butcher_computer::Computer_memory::compute_memory_usage_output(node) * 8 /
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
  import_weights_aMLLibrary_local_original(graph_type &graph, const network_butcher_parameters::Parameters &params)
  {
#if PYBIND_ACTIVE

    if (params.devices.size() == 2 && !params.backward_connections_allowed)
      {
        pybind11::initialize_interpreter();

        auto const macs     = read_network_info_onnx_tool(network_info_onnx_tool(params));
        auto const csv_path = "aMLLibrary_input.csv";

        if (!Utilities::directory_exists(params.temporary_directory))
          Utilities::create_directory(params.temporary_directory);

        if (Utilities::file_exists(csv_path))
          Utilities::file_delete(csv_path);

        std::vector<std::vector<std::string>> aMLLibrary_input;
        aMLLibrary_input.reserve(graph.size() + 1);

        aMLLibrary_input.push_back(Utilities::trim_copy(params.weight_csv_features));
        aMLLibrary_input.front().insert(aMLLibrary_input.front().cend(),
                                        params.weight_inference_variables.cbegin(),
                                        params.weight_inference_variables.cend());

        for (auto const &node : graph.get_nodes())
          {
            auto                     info_it = macs.find(node.name);
            std::vector<std::string> row;
            row.reserve(aMLLibrary_input.front().size());

            if (info_it != macs.cend())
              {
                for (auto const &entry : aMLLibrary_input.front())
                  {
                    row.emplace_back(aMLLibrary_original_generate_csv_entry(entry, info_it->second, node, params));
                  }
              }
            else
              {
                for (auto const &entry : aMLLibrary_input.front())
                  {
                    row.emplace_back(aMLLibrary_original_generate_csv_entry(entry, node, params));
                  }
              }

            aMLLibrary_input.push_back(std::move(row));
          }

        csv_assembler(aMLLibrary_input, csv_path);

        add_aMLLibrary_to_path();

        for (std::size_t i = 0; i < params.devices.size(); ++i)
          {
            std::string basic = "predict_" + std::to_string(i);
            execute_weight_generator(params.devices[i].weights_path,
                                     basic + ".ini",
                                     Utilities::combine_path(params.temporary_directory, basic),
                                     params.package_aMLLibrary_location);

            import_weights_aMLLibrary_direct_read(
              graph,
              params.devices[i].id,
              Utilities::combine_path(Utilities::combine_path(params.temporary_directory, basic), "prediction.csv"));
          }


        pybind11::finalize_interpreter();
      }
    else
      {
        std::cout << "aMLLibrary_local works only with two devices!" << std::endl;
      }

#else
    std::cout << "PyBind should be turned on in order to produce locally the weights!" << std::endl;
#endif
  }


  std::vector<std::string>
  aMLLibrary_block_generate_csv_entry(std::vector<std::string> const                &entries,
                                      network_butcher_parameters::Parameters const  &params,
                                      block_graph_type const                        &new_graph,
                                      graph_type const                              &graph,
                                      std::size_t                                    id,
                                      std::map<std::string, onnx_tool_output> const &map_onnx_tool)
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
                    mem += network_butcher_computer::Computer_memory::compute_memory_usage_output(graph[node_id]);
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
              mem += network_butcher_computer::Computer_memory::compute_memory_usage_output(graph[node_id]);

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
  import_weights_aMLLibrary_local_block(block_graph_type                             &new_graph,
                                        graph_type const                             &graph,
                                        network_butcher_parameters::Parameters const &params)
  {
#if PYBIND_ACTIVE

    if (params.devices.size() == 2 && !params.backward_connections_allowed)
      {
        pybind11::initialize_interpreter();

        auto const macs     = read_network_info_onnx_tool(network_info_onnx_tool(params));
        auto const csv_path = "aMLLibrary_input.csv";

        if (!Utilities::directory_exists(params.temporary_directory))
          Utilities::create_directory(params.temporary_directory);

        if (Utilities::file_exists(csv_path))
          Utilities::file_delete(csv_path);

        std::vector<std::vector<std::string>> aMLLibrary_input;
        aMLLibrary_input.reserve(new_graph.size() + 1);
        aMLLibrary_input.push_back(Utilities::trim_copy(params.weight_csv_features));
        aMLLibrary_input.front().insert(aMLLibrary_input.front().cend(),
                                        params.weight_inference_variables.cbegin(),
                                        params.weight_inference_variables.cend());

        for (auto const &node : new_graph.get_nodes())
          {
            if (node.content.first != 0 ||
                !node.content.second->empty() && (graph[*node.content.second->cbegin()].name == "__fake__input__" ||
                                                  graph[*node.content.second->cbegin()].name == "__fake__output__"))
              continue;

            aMLLibrary_input.push_back(aMLLibrary_block_generate_csv_entry(
              aMLLibrary_input.front(), params, new_graph, graph, node.get_id(), macs));
          }


        csv_assembler(aMLLibrary_input, csv_path);

        add_aMLLibrary_to_path();

        for (std::size_t i = 0; i < params.devices.size(); ++i)
          {
            std::string basic = "predict_" + std::to_string(i);
            execute_weight_generator(params.devices[i].weights_path,
                                     basic + ".ini",
                                     Utilities::combine_path(params.temporary_directory, basic),
                                     params.package_aMLLibrary_location);

            import_weights_aMLLibrary_direct_read(
              new_graph,
              params.devices[i].id,
              params.devices.size(),
              Utilities::combine_path(Utilities::combine_path(params.temporary_directory, basic), "prediction.csv"));
          }

        pybind11::finalize_interpreter();
      }
    else
      {
        std::cout << "aMLLibrary_local works only with two devices!" << std::endl;
      }

#else
    std::cout << "PyBind should be turned on in order to produce locally the weights!" << std::endl;
#endif
  }


  void
  import_weights_official_csv_multi_operation_time(graph_type              &graph,
                                                   std::vector<std::size_t> devices,
                                                   const std::string       &path)
  {
    std::fstream file_in;
    file_in.open(path, std::ios_base::in);
    weight_type tmp_weight;


    auto it = graph.get_nodes().cbegin();

    std::string tmp_line;
    std::getline(file_in, tmp_line);

    std::map<std::size_t, Index_Type> indices;

    {
      std::stringstream stream_line(tmp_line);
      std::size_t       j = 0;
      while (std::getline(stream_line, tmp_line, ','))
        {
          Utilities::to_lowercase(tmp_line);

          if (tmp_line.find("optype") != std::string::npos)
            {
              indices[j] = Index_Type::Operation;
            }
          else if (tmp_line.find("cloud") != std::string::npos)
            {
              indices[j] = Index_Type::Cloud;
            }
          else if (tmp_line.find("edge") != std::string::npos)
            {
              indices[j] = Index_Type::Edge;
            }

          ++j;
        }

      if (devices.size() + 1 > indices.size())
        {
          std::cout << "Missing weights" << std::endl;
          return;
        }
    }

    while (!file_in.eof())
      {
        std::list<weight_type> tmp_weights;

        std::getline(file_in, tmp_line);

        if (tmp_line.empty())
          continue;

        std::stringstream stream_line(tmp_line);

        std::getline(stream_line, tmp_line, ',');
        std::size_t jj = 0;

        for (auto indices_it = indices.cbegin(); indices_it != indices.cend(); ++indices_it)
          {
            while (jj < indices_it->first)
              {
                std::getline(stream_line, tmp_line, ',');
                ++jj;
              }

            switch (indices_it->second)
              {
                  case Operation: {
                    while (it != graph.get_nodes().cend() &&
                           Utilities::to_lowercase_copy(it->content.get_operation_id()) !=
                             Utilities::to_lowercase_copy(tmp_line))
                      {
                        ++it;
                      }
                    break;
                  }
                  case Cloud: {
                    tmp_weights.emplace_back(::atof(tmp_line.c_str()));
                    break;
                  }
                  case Edge: {
                    tmp_weights.emplace_back(::atof(tmp_line.c_str()));
                    break;
                  }
                  default: {
                    std::cout << "Missing weights" << std::endl;
                    return;
                  }
              }
          }

        if (it == graph.get_nodes().cend())
          {
            std::cout << "Cannot find weight" << std::endl;
            return;
          }

        std::size_t j = 0;
        for (auto tmp_weights_it = tmp_weights.cbegin(); tmp_weights_it != tmp_weights.cend() && j < devices.size();
             ++tmp_weights_it, ++j)
          {
            for (auto const &successor : graph.get_neighbors()[it->get_id()].second)
              {
                graph.set_weight(devices[j], {it->get_id(), successor}, tmp_weight);
              }
          }

        ++it;
      }
  }


  void
  import_weights_custom_csv_multi_operation_time(graph_type              &graph,
                                                 std::vector<std::size_t> devices,
                                                 const std::string       &path)
  {
    // Import the file
    std::fstream file_in;
    file_in.open(path, std::ios_base::in);
    weight_type tmp_weight;

    auto it = graph.get_nodes().cbegin();

    std::string tmp_line;
    std::getline(file_in, tmp_line);

    // If not the end of file,
    while (!file_in.eof())
      {
        std::getline(file_in, tmp_line);
        std::stringstream stream_line(tmp_line);

        std::getline(stream_line, tmp_line, ',');
        auto const operation_name = tmp_line;

        while (it != graph.get_nodes().cend() && Utilities::to_lowercase_copy(it->content.get_operation_id()) !=
                                                   Utilities::to_lowercase_copy(operation_name))
          {
            ++it;
          }

        if (it == graph.get_nodes().cend())
          return;

        std::size_t j = 0;
        while (std::getline(stream_line, tmp_line, ','))
          {
            tmp_weight = ::atof(tmp_line.c_str());

            for (auto const &successor : graph.get_neighbors()[it->get_id()].second)
              {
                graph.set_weight(devices[j], {it->get_id(), successor}, tmp_weight);
              }

            ++it;
            ++j;
          }
      }

    file_in.close();
  }


#if PYBIND_ACTIVE

  void
  add_aMLLibrary_to_path()
  {
    using namespace pybind11::literals;
    namespace py = pybind11;

    py::object path     = py::module_::import("sys").attr("path");
    py::object inserter = path.attr("append");
    inserter(NN_Source_Path);
  }


  void
  execute_weight_generator(const std::string &regressor_file,
                           const std::string &config_file,
                           const std::string &output_path,
                           const std::string &package_path)
  {
    using namespace pybind11::literals;
    namespace py = pybind11;

    if (Utilities::directory_exists(output_path))
      Utilities::directory_delete(output_path);

    py::object Predictor = py::module_::import("aMLLibrary.model_building.predictor").attr("Predictor");
    py::object predict =
      Predictor("regressor_file"_a = regressor_file, "output_folder"_a = output_path, "debug"_a = false)
        .attr("predict");

    predict("config_file"_a = config_file, "mape_to_file"_a = false);
  }


  std::string
  network_info_onnx_tool(const std::string &model_path,
                         const std::string &package_onnx_tool_location,
                         const std::string &temporary_directory)
  {
    using namespace pybind11::literals;
    namespace py = pybind11;

    if (!package_onnx_tool_location.empty())
      {
        py::object path     = py::module_::import("sys").attr("path");
        py::object inserter = path.attr("append");
        inserter(package_onnx_tool_location);
      }

    if (!Utilities::directory_exists(temporary_directory))
      Utilities::create_directory(temporary_directory);

    auto weight_path = Utilities::combine_path(temporary_directory, "weights.csv");
    if (Utilities::file_exists(weight_path))
      Utilities::file_delete(weight_path);

    py::object onnx_tool = py::module_::import("onnx_tool");

    py::object model_profile = onnx_tool.attr("model_profile");
    model_profile(model_path, "savenode"_a = weight_path);

    return weight_path;
  }


  std::string
  network_info_onnx_tool(const network_butcher_parameters::Parameters &params)
  {
    return network_info_onnx_tool(params.model_path, params.package_onnx_tool_location, params.temporary_directory);
  }


  std::map<std::string, onnx_tool_output>
  read_network_info_onnx_tool(const std::string &path)
  {
    std::map<std::string, onnx_tool_output> res;

    // Import the file
    std::fstream file_in;
    file_in.open(path, std::ios_base::in);
    weight_type tmp_weight;

    std::string tmp_line;
    std::getline(file_in, tmp_line);

    // If not the end of file,
    while (!file_in.eof())
      {
        std::string      operation;
        onnx_tool_output info;

        std::getline(file_in, tmp_line);
        std::stringstream stream_line(tmp_line);

        std::getline(stream_line, tmp_line, ','); // name
        info.name = Utilities::to_lowercase_copy(std::move(tmp_line));

        std::getline(stream_line, tmp_line, ','); // macs
        {
          std::stringstream stream(tmp_line);
          stream >> info.macs;
        }

        std::getline(stream_line, tmp_line, ','); // CPercent
        std::getline(stream_line, tmp_line, ','); // Memory

        {
          std::stringstream stream(tmp_line);
          stream >> info.memory;
        }

        std::getline(stream_line, tmp_line, ','); // MPercent
        std::getline(stream_line, tmp_line, ','); // Params

        {
          std::stringstream stream(tmp_line);
          stream >> info.params;
        }

        res.emplace(info.name, std::move(info));
      }

    file_in.close();

    return res;
  }

#endif

} // namespace network_butcher_io::Weight_importer_helpers