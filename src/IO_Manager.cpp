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
  import_from_onnx(std::string const &path,
                   bool               add_input_padding,
                   bool               add_output_padding,
                   std::size_t        num_devices,
                   bool               unused_ios)
  {
    using namespace network_butcher::io::Onnx_importer_helpers;

    // Parse from the file the onnx::ModelProto
    onnx::ModelProto onnx_model = network_butcher::Utilities::parse_onnx_file(path);

    // Simple renaming
    auto const &onnx_graph = onnx_model.graph();
    auto const &onnx_nodes = onnx_graph.node();

    // Prepare for the import
    auto const basic_data = prepare_import_from_onnx(onnx_graph, add_input_padding, add_output_padding, unused_ios);

    std::size_t total_size = onnx_nodes.size();

    std::vector<node_type> nodes;
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
        // tt["__fake__input__"] = basic_data.pointer_input;

        for (auto const &in : graph_inputs)
          tt.emplace(in->get_name(), in);

        nodes.emplace(nodes.begin(), network_butcher::types::Content<type_info_pointer>({}, std::move(tt)));
        nodes.front().name = "__fake__input__";
      }

    std::map<node_id_type, node_id_type> link_id_nodeproto;
    for (std::size_t i = add_input_padding ? 1 : 0, onnx_node_id = 0; i < nodes.size(); ++i, ++onnx_node_id)
      link_id_nodeproto.emplace_hint(link_id_nodeproto.end(), i, onnx_node_id);

    // If add_output_padding, then we will add a "fake" output node
    if (add_output_padding)
      {
        io_collection_type<type_info_pointer> tt;
        // tt["__fake__output__"] = basic_data.pointer_output;

        for (auto const &out : graph_outputs)
          tt.emplace(out->get_name(), out);

        nodes.emplace_back(network_butcher::types::Content<type_info_pointer>(std::move(tt)));
        nodes.back().name = "__fake__output__";
      }

    return {network_butcher::types::MWGraph(num_devices, nodes), onnx_model, link_id_nodeproto};
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

    res.model_name          = file(basic_infos + "/model_name", "model");
    res.model_path          = file(basic_infos + "/model_path", "");
    res.export_directory    = file(basic_infos + "/export_directory", "ksp_result");
    res.temporary_directory = file(basic_infos + "/temporary_directory", "tmp");

    {
      auto const len = file.vector_variable_size(basic_infos + "/extra_packages_location");
      res.extra_packages_location.reserve(len);
      for (std::size_t i = 0; i < len; ++i)
        res.extra_packages_location.emplace_back(file(basic_infos + "/extra_packages_location", i, ""));
    }

    res.K                    = file(basic_infos + "/K", 100);
    std::string const method = network_butcher::Utilities::trim_copy(
      network_butcher::Utilities::to_lowercase_copy(file(basic_infos + "/method", "")));

    if (method == "eppstein")
      res.method = network_butcher::parameters::KSP_Method::Eppstein;
    else
      res.method = network_butcher::parameters::KSP_Method::Lazy_Eppstein;

    res.starting_device_id = file(basic_infos + "/starting_device_id", 0);
    res.ending_device_id   = file(basic_infos + "/ending_device_id", 0);

    res.backward_connections_allowed = file(basic_infos + "/backward_connections_allowed", false);

    std::string const weight_import_method = network_butcher::Utilities::trim_copy(
      network_butcher::Utilities::to_lowercase_copy(file(weight_infos + "/import_mode", "")));

    if (weight_import_method == "amllibrary_original")
      {
        res.weight_import_mode = network_butcher::parameters::Weight_Import_Mode::aMLLibrary_original;
      }
    else if (weight_import_method == "amllibrary_block")
      {
        res.weight_import_mode = network_butcher::parameters::Weight_Import_Mode::aMLLibrary_block;
      }
    else if (weight_import_method == "single_direct_read")
      {
        res.weight_import_mode = network_butcher::parameters::Weight_Import_Mode::single_direct_read;
      }
    else if (weight_import_method == "multiple_direct_read")
      {
        res.weight_import_mode = network_butcher::parameters::Weight_Import_Mode::multiple_direct_read;
      }
    else if (weight_import_method == "block_single_direct_read")
      {
        res.weight_import_mode = network_butcher::parameters::Weight_Import_Mode::block_single_direct_read;
      }
    else if (weight_import_method == "block_multiple_direct_read")
      {
        res.weight_import_mode = network_butcher::parameters::Weight_Import_Mode::block_multiple_direct_read;
      }
    else
      {
        throw std::logic_error("Unavailable weight import mode!");
      }

    std::string const block_graph_mode = network_butcher::Utilities::trim_copy(
      network_butcher::Utilities::to_lowercase_copy(file(basic_infos + "/block_graph_mode", "classic")));

    if (block_graph_mode == "classic")
      {
        res.block_graph_mode = network_butcher::parameters::Block_Graph_Generation_Mode::classic;
      }
    else if (block_graph_mode == "input")
      {
        res.block_graph_mode = network_butcher::parameters::Block_Graph_Generation_Mode::input;
      }
    else if (block_graph_mode == "output")
      {
        res.block_graph_mode = network_butcher::parameters::Block_Graph_Generation_Mode::output;
      }
    else
      {
        throw std::logic_error("Unavailable Block Graph generation mode!");
      }


    {
      auto const len = file.vector_variable_size(weight_infos + "/aMLLibrary_inference_variables");
      res.aMLLibrary_inference_variables.reserve(len);
      for (std::size_t i = 0; i < len; ++i)
        res.aMLLibrary_inference_variables.emplace_back(file(weight_infos + "/aMLLibrary_inference_variables", i, ""));
    }

    {
      auto const len = file.vector_variable_size(weight_infos + "/aMLLibrary_features");
      res.aMLLibrary_csv_features.reserve(len);
      for (std::size_t i = 0; i < len; ++i)
        res.aMLLibrary_csv_features.emplace_back(file(weight_infos + "/aMLLibrary_features", i, ""));
    }

    {
      auto const len = file.vector_variable_size(weight_infos + "/single_csv_columns_weights");
      res.single_csv_columns_weights.reserve(len);
      for (std::size_t i = 0; i < len; ++i)
        res.single_csv_columns_weights.emplace_back(file(weight_infos + "/single_csv_columns_weights", i, ""));

      network_butcher::Utilities::trim(res.single_csv_columns_weights);
      network_butcher::Utilities::to_lowercase(res.single_csv_columns_weights);
    }

    res.single_weight_import_path = file(weight_infos + "/single_weight_import_path", "");
    res.separator                 = file(weight_infos + "/separator", ',');

    res.memory_constraint = file(basic_infos + "/memory_constraint", false);
    if (res.memory_constraint)
      {
        std::string const memory_constraint_type = network_butcher::Utilities::trim_copy(
          network_butcher::Utilities::to_lowercase_copy(file(basic_infos + "/memory_constraint_type", "none")));

        if (memory_constraint_type == "max")
          {
            res.memory_constraint_type = network_butcher::parameters::Memory_Constraint_Type::Max;
          }
        else if (memory_constraint_type == "preload_parameters")
          {
            res.memory_constraint_type = network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters;
          }
        else
          {
            res.memory_constraint_type = network_butcher::parameters::Memory_Constraint_Type::None;
          }
      }


    std::size_t num_devices = file(basic_infos + "/num_devices", 1);
    res.devices.reserve(num_devices);

    for (std::size_t i = 0; i < num_devices; ++i)
      {
        std::string const prx = "device_" + std::to_string(i);

        network_butcher::parameters::Device dev;

        dev.id             = i;
        dev.name           = file(prx + "/name", "");
        dev.maximum_memory = file(prx + "/maximum_memory", 0);
        dev.weights_path   = file(prx + "/path", "");
        dev.relevant_entry = file(prx + "/relevant_entry", "");

        network_butcher::Utilities::trim(dev.relevant_entry);
        network_butcher::Utilities::to_lowercase(dev.relevant_entry);

        res.devices.push_back(std::move(dev));
      }

    std::string const bndw = "bandwidth";
    std::string const accc = "access_delay";
    for (std::size_t i = 0; i < num_devices; ++i)
      {
        for (std::size_t j = i + 1; j < num_devices; ++j)
          {
            auto const basic      = "/from_" + std::to_string(i) + "_to_" + std::to_string(j);
            res.bandwidth[{i, j}] = {file(bndw + basic, .0), file(accc + basic, .0)};
          }
      }

    if (res.backward_connections_allowed)
      {
        for (std::size_t i = num_devices; i >= 0; --i)
          {
            for (std::size_t j = i - 1; j >= 0; --j)
              {
                auto const basic      = "/from_" + std::to_string(i) + "_to_" + std::to_string(j);
                res.bandwidth[{i, j}] = {file(bndw + basic, .0), file(accc + basic, .0)};
              }
          }
      }

    return res;
  }


  void
  export_network_partitions(const network_butcher::parameters::Parameters     &params,
                            onnx::ModelProto const                            &model,
                            std::map<node_id_type, node_id_type> const        &link_id_nodeproto,
                            const network_butcher::types::Weighted_Real_Paths &paths)
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
    switch (params.weight_import_mode)
      {
          case network_butcher::parameters::Weight_Import_Mode::aMLLibrary_original: {
            return std::make_unique<original_aMLLibrary_Weight_Importer>(
              original_aMLLibrary_Weight_Importer(graph, params));
          }
          case network_butcher::parameters::Weight_Import_Mode::single_direct_read: {
            return std::make_unique<Csv_Weight_Importer<graph_type>>(
              Csv_Weight_Importer(graph,
                                  {params.single_weight_import_path},
                                  params.single_csv_columns_weights,
                                  params.devices,
                                  params.separator));
          }
          case network_butcher::parameters::Weight_Import_Mode::multiple_direct_read: {
            return std::make_unique<Csv_Weight_Importer<graph_type>>(
              Csv_Weight_Importer(graph, params.devices, params.separator));
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

    if (params.weight_import_mode == Weight_Import_Mode::aMLLibrary_block ||
        params.weight_import_mode == Weight_Import_Mode::block_multiple_direct_read ||
        params.weight_import_mode == Weight_Import_Mode::block_single_direct_read)
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

                        params.bandwidth[{i, j}] = Yaml_importer_helpers::find_bandwidth(network_domains,
                                                                                         subdomain_to_domain,
                                                                                         first_domain,
                                                                                         second_domain);
                      }
                  }
              }
          }
      }

    return res;
  }

} // namespace network_butcher::io::IO_Manager

#endif