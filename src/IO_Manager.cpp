//
// Created by faccus on 20/02/22.
//

#include "IO_Manager.h"

namespace network_butcher_io::IO_Manager
{
  void
  utilities::reconstruct_model_and_export(
    const network_butcher_types::Real_Path     &partitions,
    const onnx::ModelProto                     &original_model,
    const std::map<node_id_type, node_id_type> &link_id_nodeproto,
    std::unordered_map<
      std::string,
      std::pair<network_butcher_io::Onnx_model_reconstructor_helpers::IO_Type,
                std::pair<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator,
                          google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>>> const
                      &preprocessed_ios_nodes,
    const std::string &export_base_path)
  {
    auto const &model_graph = original_model.graph();

    for (std::size_t i = 0; i < partitions.size(); ++i)
      {
        auto const &partition       = partitions[i];
        auto const  partition_model = reconstruct_model_from_partition(
          partition, original_model, link_id_nodeproto, preprocessed_ios_nodes, model_graph);

        if (partition_model.first)
          {
            export_to_onnx(partition_model.second,
                           export_base_path + "-" + std::to_string(i) + "-device-" + std::to_string(partition.first) +
                             ".onnx");
          }
      }
  }


  std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
  import_from_onnx(std::string const &path,
                   bool               add_input_padding,
                   bool               add_output_padding,
                   std::size_t        num_devices,
                   bool               unused_ios)
  {
    std::map<node_id_type, node_id_type> link_id_nodeproto;

    // Parse from the file the onnx::ModelProto
    onnx::ModelProto onnx_model = Utilities::parse_onnx_file(path);

    // Simple renaming
    auto const &onnx_graph = onnx_model.graph();
    auto const &onnx_nodes = onnx_graph.node();

    // Generate value_infos and the ids (names) of the inputs and outputs
    auto [value_infos, onnx_inputs_ids, onnx_outputs_ids] = Onnx_importer_helpers::compute_value_infos(
      onnx_graph.input(), onnx_graph.output(), onnx_graph.value_info(), onnx_graph.initializer());

    // Prepare the node vector for the graph
    std::vector<node_type> nodes;
    nodes.reserve(onnx_nodes.size() + 2);

    auto const pointer_output = std::make_shared<network_butcher_types::Dense_tensor>(0, std::vector<shape_type>{});
    auto const pointer_input  = std::make_shared<network_butcher_types::Dense_tensor>(0, std::vector<shape_type>{});

    node_id_type node_id      = 0;
    node_id_type onnx_node_id = 0;

    std::set<std::string> unused_ios_set;
    if (unused_ios)
      unused_ios_set = Onnx_importer_helpers::find_unused_ios(onnx_graph);

    // If add_input_padding, then we will add a "fake" input node
    if (add_input_padding)
      {
        io_collection_type<type_info_pointer> tt;
        tt["__fake__input__"] = pointer_input;

        nodes.emplace_back(network_butcher_types::Content<type_info_pointer>({}, std::move(tt)));
        nodes.front().name = "__fake__input__";
        ++node_id;
      }

    // Populate every node
    for (auto const &node : onnx_nodes)
      {
        auto operation_type = Utilities::to_lowercase_copy(node.op_type());

        io_collection_type<type_info_pointer> parameters;
        auto inputs  = Onnx_importer_helpers::process_node_ios(node.input(), parameters, value_infos, unused_ios_set);
        auto outputs = Onnx_importer_helpers::process_node_ios(node.output(), parameters, value_infos, unused_ios_set);

        // If the inputs of the node are the inputs of the NN, then add the connection with the padding node
        if (add_input_padding && !Onnx_importer_helpers::get_common_elements(onnx_inputs_ids, inputs).empty())
          {
            inputs["__fake__input__"] = pointer_input;
          }

        // If the inputs of the node are the outputs of the NN, then add the connection with the padding node
        if (add_output_padding && !Onnx_importer_helpers::get_common_elements(onnx_outputs_ids, outputs).empty())
          {
            outputs["__fake__output__"] = pointer_output;
          }

        auto content = network_butcher_types::Content<type_info_pointer>::make_content(
          std::move(inputs),
          std::move(outputs),
          std::move(parameters),
          Onnx_importer_helpers::process_node_attributes(node),
          std::move(operation_type));
        nodes.emplace_back(std::move(content));
        // nodes.back().name = Utilities::to_lowercase_copy(node.op_type() + "_" + std::to_string(onnx_node_id));
        nodes.back().name = node.name();

        link_id_nodeproto.emplace(node_id++, onnx_node_id++);
      }

    // If add_output_padding, then we will add a "fake" output node
    if (add_output_padding)
      {
        io_collection_type<type_info_pointer> tt;
        tt["__fake__output__"] = pointer_output;

        nodes.emplace_back(network_butcher_types::Content<type_info_pointer>(std::move(tt)));
        nodes.back().name = "__fake__output__";
        ++node_id;
      }

    return {network_butcher_types::MWGraph(num_devices, nodes), onnx_model, link_id_nodeproto};
  }


  void
  export_to_onnx(const onnx::ModelProto &model, std::string path)
  {
    Utilities::output_onnx_file(model, path);
  }


  void
  old_export_network_infos_to_csv(graph_type const &graph, onnx::ModelProto const &model, std::string const &path)
  {
    std::fstream file_out;
    file_out.open(path, std::ios_base::out);

    file_out << "Layer,Hf,Wf,Cin,Cout,FLOPS,Time" << std::endl;

    for (auto const &node : graph.get_nodes())
      {
        if (node.content.get_operation_id() == "conv")
          {
            auto const &content = node.content;

            auto       ins             = content.get_input();
            auto       outs            = content.get_output();
            auto const kernel_iterator = content.get_attributes().find("kernel_shape");

            if (kernel_iterator != content.get_attributes().cend())
              {
                auto out_it = outs.cbegin();
                while (out_it != outs.cend() && out_it->first == "__fake__output__")
                  ++out_it;

                auto in_it = ins.cbegin();
                while (in_it != ins.cend() && in_it->first == "__fake__input__")
                  ++in_it;

                if (in_it == ins.cend() || out_it == outs.cend())
                  continue;

                auto const &out_shape    = out_it->second->get_shape();
                auto const &in_shape     = in_it->second->get_shape();
                auto const &kernel_shape = kernel_iterator->second.get_ints();

                auto const &H_f = kernel_shape[0];
                auto const &W_f = kernel_shape[1];

                std::size_t const C_in      = in_shape[1];
                std::size_t const C_out     = out_shape[1];
                std::size_t const in_pixels = in_shape[2] * in_shape[3];

                auto const flops = H_f * W_f * C_in * C_out * in_pixels;

                file_out << node.get_id() << "," << H_f << "," << W_f << "," << C_in << "," << C_out << "," << flops
                         << ",0" << std::endl;
              }
          }
      }

    file_out.close();
  }


  void
  export_network_infos_to_csv(graph_type const &graph, std::string const &path)
  {
    std::fstream file_out;
    file_out.open(path, std::ios_base::out);

    file_out << "Layer,Hf,Wf,Cin,Cout,FLOPS,Time" << std::endl;

    for (auto const &node : graph.get_nodes())
      {
        if (node.content.get_operation_id() == "conv")
          {
            auto const &content = node.content;

            auto       ins             = content.get_input();
            auto       outs            = content.get_output();
            auto const kernel_iterator = content.get_attributes().find("kernel_shape");

            if (kernel_iterator != content.get_attributes().cend())
              {
                auto out_it = outs.cbegin();
                while (out_it != outs.cend() && out_it->first == "__fake__output__")
                  ++out_it;

                auto in_it = ins.cbegin();
                while (in_it != ins.cend() && in_it->first == "__fake__input__")
                  ++in_it;

                if (in_it == ins.cend() || out_it == outs.cend())
                  continue;

                auto const &out_shape    = out_it->second->get_shape();
                auto const &in_shape     = in_it->second->get_shape();
                auto const &kernel_shape = kernel_iterator->second.get_ints();

                auto const &H_f = kernel_shape[0];
                auto const &W_f = kernel_shape[1];

                std::size_t const C_in      = in_shape[1];
                std::size_t const C_out     = out_shape[1];
                std::size_t const in_pixels = in_shape[2] * in_shape[3];

                auto const flops = H_f * W_f * C_in * C_out * in_pixels;

                file_out << node.get_id() << "," << H_f << "," << W_f << "," << C_in << "," << C_out << "," << flops
                         << ",0" << std::endl;
              }
          }
      }

    file_out.close();
  }


  network_butcher_parameters::Parameters
  read_parameters(const std::string &path)
  {
    GetPot file(path);

    network_butcher_parameters::Parameters res;
    std::string const                      basic_infos  = "basic_config";
    std::string const                      weight_infos = "weight_config";

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
    std::string const method = Utilities::trim_copy(Utilities::to_lowercase_copy(file(basic_infos + "/method", "")));

    if (method == "Eppstein")
      res.method = network_butcher_parameters::KSP_Method::Eppstein;
    else
      res.method = network_butcher_parameters::KSP_Method::Lazy_Eppstein;

    res.starting_device_id = file(basic_infos + "/starting_device_id", 0);
    res.ending_device_id   = file(basic_infos + "/ending_device_id", 0);

    res.backward_connections_allowed = file(basic_infos + "/backward_connections_allowed", false);

    std::string const weight_import_method =
      Utilities::trim_copy(Utilities::to_lowercase_copy(file(weight_infos + "/import_mode", "")));


    if (weight_import_method == "amllibrary_local_inference_original")
      res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::aMLLibrary_inference_original;
    else if (weight_import_method == "amllibrary_local_inference_block")
      res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::aMLLibrary_inference_block;
    else if (weight_import_method == "single_direct_read")
      res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::single_direct_read;
    else if (weight_import_method == "multiple_direct_read")
      res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::multiple_direct_read;
    else
      std::cout << "Unavaible weight import mode!" << std::endl;


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

      Utilities::trim(res.single_csv_columns_weights);
      Utilities::to_lowercase(res.single_csv_columns_weights);
    }

    res.single_weight_import_path = file(weight_infos + "/single_weight_import_path", "");
    res.separator                 = file(weight_infos + "/separator", ',');

    res.memory_constraint = file(basic_infos + "/memory_constraint", false);
    if (res.memory_constraint)
      {
        std::string const memory_constraint_type =
          Utilities::trim_copy(Utilities::to_lowercase_copy(file(basic_infos + "/memory_constraint_type", "none")));

        if (memory_constraint_type == "none")
          {
            res.memory_constraint_type = network_butcher_parameters::Memory_Constraint_Type::None;
          }
        else if (memory_constraint_type == "max")
          {
            res.memory_constraint_type = network_butcher_parameters::Memory_Constraint_Type::Max;
          }
        else if (memory_constraint_type == "preload_parameters")
          {
            res.memory_constraint_type = network_butcher_parameters::Memory_Constraint_Type::Preload_Parameters;
          }
      }


    std::size_t num_devices = file(basic_infos + "/num_devices", 1);
    res.devices.reserve(num_devices);

    for (std::size_t i = 0; i < num_devices; ++i)
      {
        std::string const prx = "device_" + std::to_string(i);

        network_butcher_parameters::Device dev;

        dev.id             = i;
        dev.name           = file(prx + "/name", "");
        dev.maximum_memory = file(prx + "/maximum_memory", 0);
        dev.weights_path   = file(prx + "/path", "");
        dev.relevant_entry = file(prx + "/relevant_entry", "");

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
  export_network_partitions(const network_butcher_parameters::Parameters     &params,
                            onnx::ModelProto const                           &model,
                            std::map<node_id_type, node_id_type> const       &link_id_nodeproto,
                            const network_butcher_types::Weighted_Real_Paths &paths)
  {
    Utilities::create_directory(params.export_directory);
    auto const preprocessed_node_ios = Onnx_model_reconstructor_helpers::process_node_ios_nodes(model.graph());

    for (std::size_t j = 0; j < paths.size(); ++j)
      {
        Utilities::create_directory(params.export_directory + "/" + std::to_string(j));

        utilities::reconstruct_model_and_export(paths[j].second,
                                                model,
                                                link_id_nodeproto,
                                                preprocessed_node_ios,
                                                params.export_directory + "/" + std::to_string(j) + "/" +
                                                  params.model_name);
      }
  }


  void
  import_weights(graph_type &graph, const network_butcher_parameters::Parameters &params)
  {
    switch (params.weight_import_mode)
      {
          case network_butcher_parameters::Weight_Import_Mode::aMLLibrary_inference_original: {
            Weight_importer_helpers::import_weights_aMLLibrary_local_original(graph, params);
            break;
          }
          case network_butcher_parameters::Weight_Import_Mode::aMLLibrary_inference_block: {
            break;
          }
          case network_butcher_parameters::Weight_Import_Mode::single_direct_read: {
            Weight_importer_helpers::import_weights_direct_read(graph,
                                                                params.single_weight_import_path,
                                                                params.devices,
                                                                params.single_csv_columns_weights,
                                                                params.separator);
            break;
          }
          case network_butcher_parameters::Weight_Import_Mode::multiple_direct_read: {
            for (std::size_t i = 0; i < params.devices.size(); ++i)
              {
                Weight_importer_helpers::import_weights_direct_read(graph,
                                                                    params.devices[i].weights_path,
                                                                    params.devices[i].id,
                                                                    params.devices[i].relevant_entry,
                                                                    params.separator);
              }
            break;
          }
        default:
          throw "The specified Weight_Import_Mode is either not avaible or not found. Please, check that you "
                "specified the correct import mode!";
          break;
      }
  }


  std::pair<bool, onnx::ModelProto>
  reconstruct_model_from_partition(
    const network_butcher_types::Real_Partition &partition,
    const onnx::ModelProto                      &original_model,
    const std::map<node_id_type, node_id_type>  &link_id_nodeproto,
    const std::unordered_map<
      std::string,
      std::pair<network_butcher_io::Onnx_model_reconstructor_helpers::IO_Type,
                std::pair<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator,
                          google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>>>
                           &preprocessed_ios_nodes,
    const onnx::GraphProto &model_graph)
  {
    onnx::ModelProto new_model;
    auto const      &node_ids = partition.second;

    if (!node_ids.empty())
      {
        network_butcher_io::Onnx_model_reconstructor_helpers::prepare_new_model(original_model, new_model);

        auto current_edited_graph =
          network_butcher_io::Onnx_model_reconstructor_helpers::prepare_new_graph(original_model);

        network_butcher_io::Onnx_model_reconstructor_helpers::add_nodes(
          link_id_nodeproto, model_graph, node_ids, current_edited_graph, preprocessed_ios_nodes);

        if (current_edited_graph->node_size() > 0)
          {
            network_butcher_io::Onnx_model_reconstructor_helpers::add_missing_inputs(original_model,
                                                                                     current_edited_graph);
            network_butcher_io::Onnx_model_reconstructor_helpers::add_missing_outputs(original_model,
                                                                                      current_edited_graph);

            new_model.set_allocated_graph(current_edited_graph);

            return {true, new_model};
          }
      }

    return {false, new_model};
  }


} // namespace network_butcher_io::IO_Manager

#if YAML_CPP_ACTIVE

namespace network_butcher_io::IO_Manager
{

  std::vector<network_butcher_parameters::Parameters>
  read_parameters_yaml(std::string const &candidate_resources_path,
                       std::string const &candidate_deployments_path,
                       std::string const &annotations_path)
  {
    std::vector<network_butcher_parameters::Parameters> res;

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

                params.method                       = network_butcher_parameters::Lazy_Eppstein;
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

} // namespace network_butcher_io::IO_Manager

#endif