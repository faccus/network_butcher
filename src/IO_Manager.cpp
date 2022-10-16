//
// Created by faccus on 20/02/22.
//

#include "../include/IO_Manager.h"


std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
network_butcher_io::IO_Manager::import_from_onnx(const std::string &path,
                                                 bool               add_padding_nodes,
                                                 std::size_t        num_devices)
{
  std::map<node_id_type, node_id_type> link_id_nodeproto;

  // Parse from the file the onnx::ModelProto
  onnx::ModelProto onnx_model = network_butcher_utilities::parse_onnx_file(path);

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

  auto const fake_output = "__fake__output__";
  auto const fake_input  = "__fake__input__";

  node_id_type node_id      = 0;
  node_id_type onnx_node_id = 0;

  // If add_padding_nodes, then we will add a "fake" input node
  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt[fake_input] = pointer_input;

      nodes.emplace_back(network_butcher_types::Content<type_info_pointer>({}, std::move(tt)));
      ++node_id;
    }

  // Populate every node
  for (auto const &node : onnx_nodes)
    {
      auto operation_type = network_butcher_utilities::to_lowercase_copy(node.op_type());

      io_collection_type<type_info_pointer> parameters;
      auto inputs  = Onnx_importer_helpers::process_node_ios(node.input(), parameters, value_infos);
      auto outputs = Onnx_importer_helpers::process_node_ios(node.output(), parameters, value_infos);

      if (add_padding_nodes)
        {
          // If the inputs of the node are the inputs of the NN, then add the connection with the padding node
          if (!Onnx_importer_helpers::get_common_elements(onnx_inputs_ids, inputs).empty())
            inputs[fake_input] = pointer_input;

          // If the inputs of the node are the outputs of the NN, then add the connection with the padding node
          if (!Onnx_importer_helpers::get_common_elements(onnx_outputs_ids, outputs).empty())
            outputs[fake_output] = pointer_output;
        }

      auto content =
        network_butcher_types::Content<type_info_pointer>::make_content(std::move(inputs),
                                                                        std::move(outputs),
                                                                        std::move(parameters),
                                                                        Onnx_importer_helpers::process_node_attributes(
                                                                          node),
                                                                        std::move(operation_type));
      nodes.emplace_back(std::move(content));

      link_id_nodeproto.emplace(node_id++, onnx_node_id++);
    }

  // If add_padding_nodes, then we will add a "fake" output node
  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt[fake_output] = pointer_output;

      nodes.emplace_back(network_butcher_types::Content<type_info_pointer>(std::move(tt)));
      ++node_id;
    }

  return {network_butcher_types::MWGraph(num_devices, nodes), onnx_model, link_id_nodeproto};
}


void
network_butcher_io::IO_Manager::export_to_onnx(const onnx::ModelProto &model, std::string path)
{
  network_butcher_utilities::output_onnx_file(model, path);
}


void
network_butcher_io::IO_Manager::export_network_infos_to_csv(graph_type const       &graph,
                                                            onnx::ModelProto const &model,
                                                            std::string const      &path)
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
              auto const &kernel_shape = kernel_iterator->second;

              auto const &H_f = kernel_iterator->second[0].get_int();
              auto const &W_f = kernel_iterator->second[1].get_int();

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
network_butcher_io::IO_Manager::import_weights_aMLLibrary(graph_type        &graph,
                                                          std::size_t        device,
                                                          std::string const &path)
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

      while (std::getline(stream_line, tmp_line, ','))
        tmp_weight = ::atof(tmp_line.c_str());

      while (it != graph.get_nodes().cend() &&
             !(it->content.get_operation_id() == "conv" &&
               it->content.get_attributes().find("kernel_shape") != it->content.get_attributes().cend()))
        {
          ++it;
        }

      if (it == graph.get_nodes().cend())
        return;

      for (auto const &successor : graph.get_dependencies()[it->get_id()].second)
        {
          graph.set_weigth(device, {it->get_id(), successor}, tmp_weight);
        }

      ++it;
    }

  file_in.close();
}


void
network_butcher_io::IO_Manager::import_weights_custom_csv_operation_time(graph_type        &graph,
                                                                         std::size_t        device,
                                                                         const std::string &path)
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
             network_butcher_utilities::trim_copy(network_butcher_utilities::to_lowercase_copy(operation_name)) !=
               network_butcher_utilities::trim_copy(
                 network_butcher_utilities::to_lowercase_copy(it->content.get_operation_id())))
        {
          ++it;
        }

      if (it == graph.get_nodes().cend())
        return;

      for (auto const &successor : graph.get_dependencies()[it->get_id()].second)
        {
          graph.set_weigth(device, {it->get_id(), successor}, tmp_weight);
        }

      ++it;
    }

  file_in.close();
}

void
network_butcher_io::IO_Manager::import_weights_official_csv_multi_operation_time(graph_type              &graph,
                                                                                 std::vector<std::size_t> device,
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
    std::size_t       jj = 0;
    while (std::getline(stream_line, tmp_line, ','))
      {
        network_butcher_utilities::to_lowercase(tmp_line);

        if (tmp_line.find("optype") != std::string::npos)
          {
            indices[jj] = Index_Type::Operation;
          }
        else if (tmp_line.find("cloud") != std::string::npos)
          {
            indices[jj] = Index_Type::Cloud;
          }
        else if (tmp_line.find("edge") != std::string::npos)
          {
            indices[jj] = Index_Type::Edge;
          }

        ++jj;
      }

    if (device.size() + 1 > indices.size())
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
                         network_butcher_utilities::to_lowercase_copy(it->content.get_operation_id()) !=
                           network_butcher_utilities::to_lowercase_copy(tmp_line))
                    {
                      ++it;
                    }
                  break;
                }
              case Cloud:
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
      for (auto tmp_weights_it = tmp_weights.cbegin(); tmp_weights_it != tmp_weights.cend() && j < device.size();
           ++tmp_weights_it, ++j)
        {
          for (auto const &successor : graph.get_dependencies()[it->get_id()].second)
            {
              graph.set_weigth(device[j], {it->get_id(), successor}, tmp_weight);
            }
        }

      ++it;
    }
}

void
network_butcher_io::IO_Manager::import_weights_custom_csv_multi_operation_time(graph_type              &graph,
                                                                               std::vector<std::size_t> device,
                                                                               const std::string       &path)
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

      while (it != graph.get_nodes().cend() &&
             network_butcher_utilities::to_lowercase_copy(it->content.get_operation_id()) !=
               network_butcher_utilities::to_lowercase_copy(operation_name))
        {
          ++it;
        }

      if (it == graph.get_nodes().cend())
        return;

      std::size_t j = 0;
      while (std::getline(stream_line, tmp_line, ','))
        {
          tmp_weight = ::atof(tmp_line.c_str());

          for (auto const &successor : graph.get_dependencies()[it->get_id()].second)
            {
              graph.set_weigth(device[j], {it->get_id(), successor}, tmp_weight);
            }

          ++it;
          ++j;
        }
    }

  file_in.close();
}


Parameters
network_butcher_io::IO_Manager::read_parameters(const std::string &path)
{
  GetPot file(path);

  Parameters        res;
  std::string const basic_infos = "basic_config";

  res.model_name       = file(basic_infos + "/model_name", "model");
  res.model_path       = file(basic_infos + "/model_path", "");
  res.export_directory = file(basic_infos + "/export_directory", "ksp_result");


  res.K                    = file(basic_infos + "/K", 100);
  std::string const method = network_butcher_utilities::trim_copy(
    network_butcher_utilities::to_lowercase_copy(file(basic_infos + "/method", "")));

  if (method == "Eppstein")
    res.method = KSP_Method::Eppstein;
  else
    res.method = KSP_Method::Lazy_Eppstein;

  res.starting_device_id = file(basic_infos + "/starting_device_id", 0);
  res.ending_device_id   = file(basic_infos + "/ending_device_id", 0);

  res.backward_connections_allowed = file(basic_infos + "/backward_connections_allowed", false);

  std::string const weight_import_method = network_butcher_utilities::trim_copy(
    network_butcher_utilities::to_lowercase_copy(file(basic_infos + "/weight_import_mode", "aMLLibrary")));

  if (weight_import_method == "amllibrary")
    res.weight_import_mode = Weight_Import_Mode::aMLLibrary;
  else if (weight_import_method == "multi_operation_time")
    res.weight_import_mode = Weight_Import_Mode::multi_operation_time;
  else if (weight_import_method == "official_operation_time")
    res.weight_import_mode = Weight_Import_Mode::official_operation_time;
  else
    res.weight_import_mode = Weight_Import_Mode::operation_time;

  res.memory_constraint = file(basic_infos + "/memory_constraint", false);
  if (res.memory_constraint)
    {
      std::string const memory_constraint_type = network_butcher_utilities::trim_copy(
        network_butcher_utilities::to_lowercase_copy(file(basic_infos + "/memory_constraint_type", "none")));

      if (memory_constraint_type == "none")
        {
          res.memory_constraint_type = Memory_Constraint_Type::None;
        }
      else if (memory_constraint_type == "max")
        {
          res.memory_constraint_type = Memory_Constraint_Type::Max;
        }
      else if (memory_constraint_type == "preload_parameters")
        {
          res.memory_constraint_type = Memory_Constraint_Type::Preload_Parameters;
        }
    }


  std::size_t num_devices = file(basic_infos + "/num_devices", 1);
  res.devices.reserve(num_devices);

  for (std::size_t i = 0; i < num_devices; ++i)
    {
      std::string const prx = "device_" + std::to_string(i);

      Device dev;

      dev.id             = i;
      dev.name           = file(prx + "/name", "");
      dev.maximum_memory = file(prx + "/maximum_memory", 0);
      dev.weights_path   = file(prx + "/weight_path", "");

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
network_butcher_io::IO_Manager::export_network_partitions(const Parameters                           &params,
                                                          graph_type const                           &graph,
                                                          onnx::ModelProto const                     &model,
                                                          std::map<node_id_type, node_id_type> const &link_id_nodeproto,
                                                          const network_butcher_types::Weighted_Real_Paths &paths)
{
  network_butcher_utilities::create_directory(params.export_directory);
  for (std::size_t j = 0; j < paths.size(); ++j)
    {
      network_butcher_utilities::create_directory(params.export_directory + "/" + std::to_string(j));

      auto const model_devices = reconstruct_model(paths[j].second, model, graph, link_id_nodeproto);

      for (std::size_t i = 0; i < model_devices.size(); ++i) {

          export_to_onnx(model_devices[i].first,
                         params.export_directory + "/" + std::to_string(j) + "/" + params.model_name + "-" +
                           std::to_string(i) + "-device-" + std::to_string(model_devices[i].second) + ".onnx");
        }
    }
}


void
network_butcher_io::IO_Manager::import_weights(Weight_Import_Mode const &weight_mode,
                                               graph_type               &graph,
                                               std::string const        &path,
                                               std::size_t               device)
{
  switch (weight_mode)
    {
      case Weight_Import_Mode::aMLLibrary:
        import_weights_aMLLibrary(graph, device, path);
        break;
      case Weight_Import_Mode::operation_time:
        import_weights_custom_csv_operation_time(graph, device, path);
        break;
      case Weight_Import_Mode::multi_operation_time:
        import_weights_custom_csv_multi_operation_time(graph, {device}, path);
        break;
      case Weight_Import_Mode::official_operation_time:
        import_weights_official_csv_multi_operation_time(graph, {device}, path);
        break;
      default:
        break;
    }
}


void
network_butcher_io::IO_Manager::import_weights(Weight_Import_Mode const &weight_mode,
                                               graph_type               &graph,
                                               std::string const        &path,
                                               std::vector<std::size_t>  devices,
                                               std::size_t               index)
{
  switch (weight_mode)
    {
      case Weight_Import_Mode::multi_operation_time:
        import_weights_custom_csv_multi_operation_time(graph, devices, path);
        break;
      case Weight_Import_Mode::aMLLibrary:
        import_weights_aMLLibrary(graph, devices[index], path);
        break;
      case Weight_Import_Mode::operation_time:
        import_weights_custom_csv_operation_time(graph, devices[index], path);
        break;
      case Weight_Import_Mode::official_operation_time:
        import_weights_official_csv_multi_operation_time(graph, devices, path);
        break;
      default:
        break;
    }
}

#if YAML_CPP_ACTIVE
std::vector<Parameters>
network_butcher_io::IO_Manager::read_parameters_yaml(std::string const &candidate_resources_path,
                                                     std::string const &candidate_deployments_path,
                                                     std::string const &annotations_path)
{
  std::vector<Parameters> res;

  // Reads the candidate_resources file and tries to construct the network domain hierarchy. Moreover, it will produce
  // the list of avaible resources
  auto [network_domains, subdomain_to_domain, devices_map] =
    Yaml_importer_helpers::read_candidate_resources(candidate_resources_path);

  // Reads the annotation file
  auto const to_deploy = Yaml_importer_helpers::read_annotations(annotations_path);

  YAML::Node components = YAML::LoadFile(candidate_deployments_path)["Components"];
  for (auto const &model : to_deploy)
    {
      auto const &[model_friendly_name, pair_ram_vram] = model;
      auto const &[model_ram, model_vram]              = pair_ram_vram;

      auto const devices_ram = Yaml_importer_helpers::read_candidate_deployments(
        components, model_friendly_name, model_ram, model_vram, devices_map);

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

              params.method                       = Lazy_Eppstein;
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
#endif