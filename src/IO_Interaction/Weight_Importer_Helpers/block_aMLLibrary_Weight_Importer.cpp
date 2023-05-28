//
// Created by faccus on 13/04/23.
//
#include "block_aMLLibrary_Weight_Importer.h"

namespace network_butcher::io
{
  std::vector<std::string>
  block_aMLLibrary_Weight_Importer::generate_entry(
    std::vector<std::string> const                                         &entries,
    std::size_t                                                             id,
    std::map<std::string, Weight_importer_helpers::onnx_tool_output> const &map_onnx_tool) const
  {
    auto const lower_case_entries = Utilities::to_lowercase_copy(entries);

    std::map<std::string, std::size_t> inserted;
    std::vector<std::string>           res;
    res.reserve(entries.size());

    auto const &original_ids    = *new_graph[id].content.second;
    auto const &node_output_ids = new_graph[*new_graph.get_output_nodes(id).cbegin()].content.second;

    for (auto const &lower_case : lower_case_entries)
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

            auto const &[band, access] = weights_params.bandwidth->get_weight(std::make_pair(0, 1));
            auto const net_time        = access + mem * static_cast<long double>(8) / (band * std::pow(10, 6));
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
          {
            res.emplace_back("");
          }
      }

    return res;
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

    add_python_packages();

    auto const macs     = read_network_info_onnx_tool(network_info_onnx_tool());
    auto const csv_path = "aMLLibrary_input.csv";

    Utilities::create_directory(aMLLibrary_params.temporary_directory);
    Utilities::file_delete(csv_path);

    // Prepare the .csv file to be fed to aMLLibrary
    std::vector<std::vector<std::string>> aMLLibrary_input;
    aMLLibrary_input.reserve(new_graph.size() + 1);
    aMLLibrary_input.push_back(Utilities::trim_copy(aMLLibrary_params.aMLLibrary_csv_features));
    aMLLibrary_input.front().insert(aMLLibrary_input.front().cend(),
                                    aMLLibrary_params.aMLLibrary_inference_variables.cbegin(),
                                    aMLLibrary_params.aMLLibrary_inference_variables.cend());

    // Generate entries for the .csv file
    for (auto const &node : new_graph.get_nodes())
      {
        if (node.content.first != 0 ||
            !node.content.second->empty() && (graph[*node.content.second->cbegin()].name == "__fake__input__" ||
                                              graph[*node.content.second->cbegin()].name == "__fake__output__"))
          continue;

        aMLLibrary_input.push_back(generate_entry(aMLLibrary_input.front(), node.get_id(), macs));
      }

    // Assemble the .csv file
    csv_assembler(aMLLibrary_input, csv_path);

    std::vector<std::string> paths;
    std::vector<std::string> relevant_entries;

    // Predict and import the weights
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

    Csv_Weight_Importer importer(new_graph, paths, relevant_entries, devices, weights_params.separator, true);
    importer.import_weights(extra_condition);
  }
} // namespace network_butcher::io