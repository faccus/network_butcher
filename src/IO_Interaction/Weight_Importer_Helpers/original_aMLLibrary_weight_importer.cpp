//
// Created by faccus on 13/04/23.
//
#include "original_aMLLibrary_weight_importer.h"

namespace network_butcher::io
{
  auto
  original_aMLLibrary_Weight_Importer::generate_entry(const std::string                          &entry,
                                                      const Converted_Onnx_Graph_Type::Node_Type &node) const
    -> std::string
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


  auto
  original_aMLLibrary_Weight_Importer::generate_entry(const std::string                               &entry,
                                                      const Weight_importer_helpers::Onnx_Tool_Output_Type &basic_info,
                                                      const Converted_Onnx_Graph_Type::Node_Type      &node) const
    -> std::string
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
        auto const &[bandwidth, access_time] = weights_params.bandwidth->get_weight(std::pair(0, 1));
        auto const net_time =
          access_time + network_butcher::computer::Computer_memory::compute_memory_usage_output(node) *
                          static_cast<long double>(8) / (bandwidth * std::pow(10, 6));
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

  auto
  original_aMLLibrary_Weight_Importer::generete_entries(
    std::vector<std::string> const                                         &entries,
    Converted_Onnx_Graph_Type::Node_Type const                             &node,
    std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type> const &map_onnx_tool) const
    -> std::vector<std::string>
  {
    auto                     info_it = map_onnx_tool.find(node.name);
    std::vector<std::string> row;
    row.reserve(entries.size());

    if (info_it != map_onnx_tool.cend())
      {
        for (auto const &entry : entries)
          {
            row.emplace_back(generate_entry(entry, info_it->second, node));
          }
      }
    else
      {
        for (auto const &entry : entries)
          {
            row.emplace_back(generate_entry(entry, node));
          }
      }

    return row;
  }


  void
  original_aMLLibrary_Weight_Importer::import_weights(
    std::function<bool(Converted_Onnx_Graph_Type::Node_Type const &)> const &extra_condition)
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
        aMLLibrary_input.push_back(generete_entries(aMLLibrary_input.front(), node, macs));
      }

    // Assemble the .csv file
    csv_assembler(aMLLibrary_input, csv_path);

    // Construct the weights through aMMLibrary
    auto [paths, relevant_entries] = perform_predictions(csv_path);

    pybind11::finalize_interpreter();

    // Import the weights
    Csv_Weight_Importer importer(graph, paths, relevant_entries, devices, weights_params.separator, true);
    importer.import_weights(extra_condition);
  }
} // namespace network_butcher::io