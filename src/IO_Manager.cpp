//
// Created by faccus on 20/02/22.
//

#include "IO_Manager.h"

namespace network_butcher::io::IO_Manager
{
  void
  utilities::reconstruct_model_and_export(network_butcher::types::Weighted_Real_Path const &weighted_path,
                                          const onnx::ModelProto                           &original_model,
                                          const std::map<node_id_type, node_id_type>       &link_id_nodeproto,
                                          preprocessed_ios_nodes_type const                &preprocessed_ios_nodes,
                                          const std::string                                &export_base_path)
  {
    auto const &model_graph = original_model.graph();

    auto const &[weight, partitions] = weighted_path;

    std::ofstream report_file(export_base_path + "-report.txt");

    report_file << "Overall path length: " << weight << std::endl;

    for (std::size_t i = 0; i < partitions.size(); ++i)
      {
        auto const &partition       = partitions[i];
        auto const  partition_model = reconstruct_model_from_partition(
          partition, original_model, link_id_nodeproto, preprocessed_ios_nodes, model_graph);

        report_file << "Device: " << partition.first << ", Nodes: ";

        for (auto const &node : partition.second)
          report_file << node << ", ";

        report_file << std::endl;

        if (partition_model.first)
          {
            export_to_onnx(partition_model.second,
                           export_base_path + "-" + std::to_string(i) + "-device-" + std::to_string(partition.first) +
                             ".onnx");
          }
      }

    report_file.close();
  }


  std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
  import_from_onnx(std::string const &path, bool add_input_padding, bool add_output_padding, std::size_t num_devices)
  {
    using namespace network_butcher::io::Onnx_importer_helpers;

    // Parse from the file the onnx::ModelProto
    onnx::ModelProto onnx_model = network_butcher::Utilities::parse_onnx_file(path);

    // Simple renaming
    auto const &onnx_graph = onnx_model.graph();
    auto const &onnx_nodes = onnx_graph.node();

    // Prepare for the import
    auto const basic_data = prepare_import_from_onnx(onnx_graph);

    std::vector<graph_type::Node_Type> nodes;
    nodes.reserve(onnx_nodes.size() + 2);

    std::set<type_info_pointer> graph_inputs, graph_outputs;

    for (auto const &onnx_node : onnx_nodes)
      {
        auto        res  = Onnx_importer_helpers::process_node(onnx_node, basic_data);
        auto const &ins  = std::get<1>(res);
        auto const &outs = std::get<2>(res);

        if (add_input_padding)
          graph_inputs.insert(ins.cbegin(), ins.cend());

        if (add_output_padding)
          graph_outputs.insert(outs.cbegin(), outs.cend());

        nodes.push_back(std::move(std::get<0>(res)));
      }


    // If add_input_padding, then we will add a "fake" input node
    if (add_input_padding)
      {
        io_collection_type<type_info_pointer> tt;

        for (auto const &in : graph_inputs)
          tt.emplace(in->get_name(), in);


        nodes.emplace(
          nodes.begin(),
          std::move(network_butcher::types::Content_Builder<type_info_pointer>().set_output(std::move(tt))).build());
        nodes.front().name = "__fake__input__";
      }

    std::map<node_id_type, node_id_type> link_id_nodeproto;
    for (std::size_t i = add_input_padding ? 1 : 0, onnx_node_id = 0; i < nodes.size(); ++i, ++onnx_node_id)
      link_id_nodeproto.emplace_hint(link_id_nodeproto.end(), i, onnx_node_id);

    // If add_output_padding, then we will add a "fake" output node
    if (add_output_padding)
      {
        io_collection_type<type_info_pointer> tt;

        for (auto const &out : graph_outputs)
          tt.emplace(out->get_name(), out);

        nodes.emplace_back(
          std::move(network_butcher::types::Content_Builder<type_info_pointer>().set_input(std::move(tt))).build());
        nodes.back().name = "__fake__output__";
      }

    return {graph_type(num_devices, nodes), onnx_model, link_id_nodeproto};
  }


  void
  export_to_onnx(const onnx::ModelProto &model, std::string path)
  {
    network_butcher::Utilities::output_onnx_file(model, path);
  }


  network_butcher::parameters::Parameters
  read_parameters(const std::string &path)
  {
    GetPot file(path);

    network_butcher::parameters::Parameters res;
    std::string const                       basic_infos  = "basic_config";
    std::string const                       weight_infos = "weight_config";


    auto const read_vector_string = [](auto &file, std::string const &section, std::string const &entry) {
      std::string const        location = section + "/" + entry;
      auto const               len      = file.vector_variable_size(location);
      std::vector<std::string> res;

      res.reserve(len);
      for (std::size_t i = 0; i < len; ++i)
        res.emplace_back(file(location, i, ""));

      return res;
    };
    auto const read_weight_import_mode_func = [weight_infos](auto &file) {
      std::string const weight_import_method = network_butcher::Utilities::trim_copy(
        network_butcher::Utilities::to_lowercase_copy(file(weight_infos + "/import_mode", "")));

      if (weight_import_method == "amllibrary_original")
        {
          return parameters::Weight_Import_Mode::aMLLibrary_original;
        }
      else if (weight_import_method == "amllibrary_block")
        {
          return parameters::Weight_Import_Mode::aMLLibrary_block;
        }
      else if (weight_import_method == "single_direct_read")
        {
          return parameters::Weight_Import_Mode::single_direct_read;
        }
      else if (weight_import_method == "multiple_direct_read")
        {
          return parameters::Weight_Import_Mode::multiple_direct_read;
        }
      else if (weight_import_method == "block_single_direct_read")
        {
          return parameters::Weight_Import_Mode::block_single_direct_read;
        }
      else if (weight_import_method == "block_multiple_direct_read")
        {
          return parameters::Weight_Import_Mode::block_multiple_direct_read;
        }
      else
        {
          throw std::logic_error("Unavailable weight import mode!");
        }
    };
    auto const read_bandwidth = [](auto &file, std::size_t num_devices, bool backward_connections_allowed) {
      std::map<edge_type, std::pair<bandwidth_type, access_delay_type>> res;

      std::string const bndw = "bandwidth";
      std::string const accc = "access_delay";
      for (std::size_t i = 0; i < num_devices; ++i)
        {
          for (std::size_t j = i + 1; j < num_devices; ++j)
            {
              auto const basic = "/from_" + std::to_string(i) + "_to_" + std::to_string(j);
              res[{i, j}]      = {file(bndw + basic, .0), file(accc + basic, .0)};
            }
        }

      if (backward_connections_allowed)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              for (std::size_t j = 0; j < i; ++j)
                {
                  auto const basic = "/from_" + std::to_string(i) + "_to_" + std::to_string(j);
                  res[{i, j}]      = {file(bndw + basic, .0), file(accc + basic, .0)};
                }
            }
        }

      return res;
    };
    auto const read_k_method = [basic_infos](auto &file) {
      std::string const method = network_butcher::Utilities::trim_copy(
        network_butcher::Utilities::to_lowercase_copy(file(basic_infos + "/method", "")));

      if (method == "eppstein")
        {
          return network_butcher::parameters::KSP_Method::Eppstein;
        }
      else if (method == "lazy_eppstein")
        {
          return network_butcher::parameters::KSP_Method::Lazy_Eppstein;
        }
      else
        {
          throw std::runtime_error("Unsupported K-shortest path method");
        }
    };
    auto const read_block_graph_mode = [basic_infos](auto &file) {
      std::string const block_graph_mode = network_butcher::Utilities::trim_copy(
        network_butcher::Utilities::to_lowercase_copy(file(basic_infos + "/block_graph_mode", "classic")));

      if (block_graph_mode == "classic")
        {
          return parameters::Block_Graph_Generation_Mode::classic;
        }
      else if (block_graph_mode == "input")
        {
          return parameters::Block_Graph_Generation_Mode::input;
        }
      else if (block_graph_mode == "output")
        {
          return parameters::Block_Graph_Generation_Mode::output;
        }
      else
        {
          throw std::logic_error("Unavailable Block Graph generation mode!");
        }
    };


    auto const aMLLibrary_params_read_func = [basic_infos, weight_infos, &read_vector_string](auto &file,
                                                                                              auto &params) {
      auto &aMLLibrary_params = params.aMLLibrary_params;

      aMLLibrary_params.temporary_directory     = file(basic_infos + "/temporary_directory", "tmp");
      aMLLibrary_params.extra_packages_location = read_vector_string(file, basic_infos, "extra_packages_location");
      aMLLibrary_params.aMLLibrary_inference_variables =
        read_vector_string(file, weight_infos, "aMLLibrary_inference_variables");
      aMLLibrary_params.aMLLibrary_csv_features = read_vector_string(file, weight_infos, "aMLLibrary_features");
    };

    auto const weights_params_func =
      [weight_infos, &read_weight_import_mode_func, &read_vector_string, &read_bandwidth](auto &file, auto &params) {
        auto &weights_params = params.weights_params;

        weights_params.weight_import_mode        = read_weight_import_mode_func(file);
        weights_params.single_weight_import_path = file(weight_infos + "/single_weight_import_path", "");
        weights_params.single_csv_columns_weights =
          read_vector_string(file, weight_infos, "single_csv_columns_weights");

        network_butcher::Utilities::trim(weights_params.single_csv_columns_weights);
        network_butcher::Utilities::to_lowercase(weights_params.single_csv_columns_weights);

        weights_params.separator = file(weight_infos + "/separator", ',');
        weights_params.bandwidth = read_bandwidth(file, params.devices.size(), params.backward_connections_allowed);
      };

    auto const basic_infos_func = [basic_infos, &read_block_graph_mode](auto &file, auto &params) {
      params.model_name       = file(basic_infos + "/model_name", "model");
      params.model_path       = file(basic_infos + "/model_path", "");
      params.export_directory = file(basic_infos + "/export_directory", "ksp_result");

      params.starting_device_id = file(basic_infos + "/starting_device_id", 0);
      params.ending_device_id   = file(basic_infos + "/ending_device_id", 0);

      params.backward_connections_allowed = file(basic_infos + "/backward_connections_allowed", false);

      params.block_graph_mode = read_block_graph_mode(file);
    };

    auto const k_shortest_path_params_func = [basic_infos, weight_infos, &read_k_method](auto &file, auto &params) {
      params.K      = file(basic_infos + "/K", 100);
      params.method = read_k_method(file);
    };

    auto const constraint_params_func = [basic_infos](auto &file, auto &params) {
      params.memory_constraint = file(basic_infos + "/memory_constraint", false);
      if (params.memory_constraint)
        {
          std::string const memory_constraint_type = network_butcher::Utilities::trim_copy(
            network_butcher::Utilities::to_lowercase_copy(file(basic_infos + "/memory_constraint_type", "none")));

          if (memory_constraint_type == "max")
            {
              params.memory_constraint_type = network_butcher::parameters::Memory_Constraint_Type::Max;
            }
          else if (memory_constraint_type == "preload_parameters")
            {
              params.memory_constraint_type = network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters;
            }
          else
            {
              params.memory_constraint_type = network_butcher::parameters::Memory_Constraint_Type::None;
            }
        }
    };

    auto const devices_params_func = [basic_infos](auto &file, auto &params) {
      std::size_t num_devices = file(basic_infos + "/num_devices", 1);
      params.devices.reserve(num_devices);

      for (std::size_t i = 0; i < num_devices; ++i)
        {
          std::string const prx = "device_" + std::to_string(i);

          network_butcher::parameters::Device dev;

          dev.id             = i;
          dev.name           = file(prx + "/name", "");
          dev.maximum_memory = file(prx + "/maximum_memory", std::size_t{0}) * 1024 * 1024;
          dev.weights_path   = file(prx + "/path", "");
          dev.relevant_entry = file(prx + "/relevant_entry", "");

          network_butcher::Utilities::trim(dev.relevant_entry);
          network_butcher::Utilities::to_lowercase(dev.relevant_entry);

          params.devices.push_back(std::move(dev));
        }
    };

    basic_infos_func(file, res);
    k_shortest_path_params_func(file, res);
    constraint_params_func(file, res);

    devices_params_func(file, res);

    weights_params_func(file, res);
    aMLLibrary_params_read_func(file, res);

    return res;
  }


  void
  export_network_partitions(const network_butcher::parameters::Parameters                 &params,
                            onnx::ModelProto const                                        &model,
                            std::map<node_id_type, node_id_type> const                    &link_id_nodeproto,
                            const std::vector<network_butcher::types::Weighted_Real_Path> &paths)
  {
    if (network_butcher::Utilities::directory_exists(params.export_directory))
      network_butcher::Utilities::directory_delete(params.export_directory);
    network_butcher::Utilities::create_directory(params.export_directory);

    Utilities::output_onnx_file(model, Utilities::combine_path(params.export_directory, params.model_name + ".onnx"));

    auto const preprocessed_node_ios = Onnx_model_reconstructor_helpers::process_node_ios_nodes(model.graph());

#if PARALLEL
    // https://stackoverflow.com/a/63340360

    std::vector<std::size_t> v(paths.size());
    std::generate(v.begin(), v.end(), [n = 0]() mutable { return n++; });

    std::for_each(PAR_UNSEQ,
                  v.cbegin(),
                  v.cend(),
                  [&params, &paths, &model, &link_id_nodeproto, &preprocessed_node_ios](std::size_t j) {
                    auto const dir_path = Utilities::combine_path(params.export_directory, std::to_string(j));
                    network_butcher::Utilities::create_directory(dir_path);

                    auto const output_path = Utilities::combine_path(dir_path, params.model_name);
                    utilities::reconstruct_model_and_export(
                      paths[j], model, link_id_nodeproto, preprocessed_node_ios, output_path);
                  });
#else
    for (std::size_t j = 0; j < paths.size(); ++j)
      {
        auto const dir_path = Utilities::combine_path(params.export_directory, std::to_string(j));
        network_butcher::Utilities::create_directory(dir_path);

        auto const output_path = Utilities::combine_path(dir_path, params.model_name);
        utilities::reconstruct_model_and_export(paths[j], model, link_id_nodeproto, preprocessed_node_ios, output_path);
      }
#endif
  }


  std::unique_ptr<Weight_Importer>
  generate_weight_importer(graph_type &graph, network_butcher::parameters::Parameters const &params)
  {
    switch (params.weights_params.weight_import_mode)
      {
          case network_butcher::parameters::Weight_Import_Mode::aMLLibrary_original: {
            return std::make_unique<original_aMLLibrary_Weight_Importer>(
              original_aMLLibrary_Weight_Importer(graph, params));
          }
          case network_butcher::parameters::Weight_Import_Mode::single_direct_read: {
            return std::make_unique<Csv_Weight_Importer<graph_type>>(
              Csv_Weight_Importer(graph,
                                  {params.weights_params.single_weight_import_path},
                                  params.weights_params.single_csv_columns_weights,
                                  params.devices,
                                  params.weights_params.separator));
          }
          case network_butcher::parameters::Weight_Import_Mode::multiple_direct_read: {
            return std::make_unique<Csv_Weight_Importer<graph_type>>(
              Csv_Weight_Importer(graph, params.devices, params.weights_params.separator));
          }
        default:
          throw std::logic_error(
            "The specified Weight_Import_Mode is either not available or not found. Please, check that you "
            "specified the correct import mode!");
      }
  }

  void
  import_weights(graph_type &graph, const network_butcher::parameters::Parameters &params)
  {
    using namespace network_butcher::parameters;

    if (params.weights_params.weight_import_mode == Weight_Import_Mode::aMLLibrary_block ||
        params.weights_params.weight_import_mode == Weight_Import_Mode::block_multiple_direct_read ||
        params.weights_params.weight_import_mode == Weight_Import_Mode::block_single_direct_read)
      {
        return;
      }

    generate_weight_importer(graph, params)->import_weights();
  }


  std::pair<bool, onnx::ModelProto>
  reconstruct_model_from_partition(const network_butcher::types::Real_Partition &partition,
                                   const onnx::ModelProto                       &original_model,
                                   const std::map<node_id_type, node_id_type>   &link_id_nodeproto,
                                   const preprocessed_ios_nodes_type            &preprocessed_ios_nodes,
                                   const onnx::GraphProto                       &model_graph)
  {
    onnx::ModelProto new_model;
    auto const      &node_ids = partition.second;

    if (!node_ids.empty())
      {
        network_butcher::io::Onnx_model_reconstructor_helpers::prepare_new_model(original_model, new_model);

        auto current_edited_graph =
          network_butcher::io::Onnx_model_reconstructor_helpers::prepare_new_graph(original_model);

        network_butcher::io::Onnx_model_reconstructor_helpers::add_nodes(
          link_id_nodeproto, model_graph, node_ids, current_edited_graph, preprocessed_ios_nodes);

        if (current_edited_graph->node_size() > 0)
          {
            network_butcher::io::Onnx_model_reconstructor_helpers::add_missing_inputs(original_model,
                                                                                      current_edited_graph);
            network_butcher::io::Onnx_model_reconstructor_helpers::add_missing_outputs(original_model,
                                                                                       current_edited_graph);

            new_model.set_allocated_graph(current_edited_graph);

            return {true, new_model};
          }
      }

    return {false, new_model};
  }


} // namespace network_butcher::io::IO_Manager

#if YAML_CPP_ACTIVE

namespace network_butcher::io::IO_Manager
{

  std::vector<network_butcher::parameters::Parameters>
  read_parameters_yaml(std::string const &candidate_resources_path,
                       std::string const &candidate_deployments_path,
                       std::string const &annotations_path)
  {
    std::vector<network_butcher::parameters::Parameters> res;

    // Reads the candidate_resources file and tries to construct the network domain hierarchy. Moreover, it will produce
    // the list of avaible resources
    auto [network_domains, subdomain_to_domain, devices_map] =
      Yaml_importer_helpers::read_candidate_resources(candidate_resources_path);

    // Reads the annotation file
    auto const to_deploy = Yaml_importer_helpers::read_annotations(annotations_path);

    // Reads the candidate deployments file
    auto const model_devices_ram =
      Yaml_importer_helpers::read_candidate_deployments(candidate_deployments_path, to_deploy, devices_map);

    for (auto const &[model_friendly_name, pair_ram_vram] : to_deploy)
      {
        auto const devices_ram = model_devices_ram.find(model_friendly_name)->second;

        // If there is more than one "feasible" device
        if (devices_ram.size() > 1)
          {
            // Prepare the collection of parameters for the current set of devices
            for (auto const &devices : Yaml_importer_helpers::get_devices_for_partitions(devices_ram))
              {
                auto &params = res.emplace_back();

                params.model_name = model_friendly_name;
                params.model_path = "";

                params.starting_device_id = 0;
                params.ending_device_id   = 0;

                params.method                       = network_butcher::parameters::KSP_Method::Lazy_Eppstein;
                params.K                            = 100;
                params.backward_connections_allowed = false;

                std::size_t k = 0;
                for (auto const &device : devices)
                  {
                    params.devices.emplace_back();

                    auto &dev          = params.devices.back();
                    dev.id             = k++;
                    dev.maximum_memory = device.second;
                    dev.name           = device.first;
                    dev.weights_path   = "";
                  }

                for (std::size_t i = 0; i < params.devices.size(); ++i)
                  {
                    for (std::size_t j = i + 1; j < params.devices.size(); ++j)
                      {
                        auto const &first_domain  = devices_map[devices[i].first].domain_name;
                        auto const &second_domain = devices_map[devices[j].first].domain_name;

                        params.weights_params.bandwidth[{i, j}] = Yaml_importer_helpers::find_bandwidth(
                          network_domains, subdomain_to_domain, first_domain, second_domain);
                      }
                  }
              }
          }
      }

    return res;
  }

} // namespace network_butcher::io::IO_Manager

#endif