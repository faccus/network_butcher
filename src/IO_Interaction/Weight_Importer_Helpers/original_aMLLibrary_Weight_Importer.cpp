//
// Created by faccus on 13/04/23.
//
#include "original_aMLLibrary_Weight_Importer.h"

namespace network_butcher::io
{
  std::string
  original_aMLLibrary_Weight_Importer::generate_entry(const std::string &entry, const graph_type::Node_Type &node) const
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
      return generate_entry(entry, {}, node);
  }


  std::string
  original_aMLLibrary_Weight_Importer::generate_entry(const std::string                               &entry,
                                                      const Weight_importer_helpers::onnx_tool_output &basic_info,
                                                      const graph_type::Node_Type                     &node) const
  {
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
        auto const net_time = weights_params.bandwidth.cbegin()->second.second +
                              network_butcher::computer::Computer_memory::compute_memory_usage_output(node) * 8 /
                                (weights_params.bandwidth.cbegin()->second.first * std::pow(10, 6));
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

    // For every node generate the relevant entry in the .csv file
    for (auto const &node : graph.get_nodes())
      {
        auto                     info_it = macs.find(node.name);
        std::vector<std::string> row;
        row.reserve(aMLLibrary_input.front().size());

        if (info_it != macs.cend())
          {
            for (auto const &entry : aMLLibrary_input.front())
              {
                row.emplace_back(generate_entry(entry, info_it->second, node));
              }
          }
        else
          {
            for (auto const &entry : aMLLibrary_input.front())
              {
                row.emplace_back(generate_entry(entry, node));
              }
          }

        aMLLibrary_input.push_back(std::move(row));
      }

    // Assemble the .csv file
    csv_assembler(aMLLibrary_input, csv_path);

    std::vector<std::string> paths;
    std::vector<std::string> relevant_entries;

    // Perform the predictions
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

    pybind11::finalize_interpreter();

    // Import the weights
    Csv_Weight_Importer importer(graph, paths, relevant_entries, devices, weights_params.separator, true);
    importer.import_weights(extra_condition);
  }
} // namespace network_butcher::io