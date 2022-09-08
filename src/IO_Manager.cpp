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
network_butcher_io::IO_Manager::import_weights_from_csv_aMLLibrary(graph_type        &graph,
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
network_butcher_io::IO_Manager::import_weights_from_csv_operation_time(graph_type        &graph,
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
network_butcher_io::IO_Manager::import_weights_from_csv_multi_operation_time(graph_type              &graph,
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

      auto const model_device = reconstruct_model(paths[j].second, model, graph, link_id_nodeproto);

      for (std::size_t i = 0; i < model_device.size(); ++i)
        export_to_onnx(model_device[i].first,
                       params.export_directory + "/" + std::to_string(j) + "/" + params.model_name + "-" +
                         std::to_string(i) + "-device-" + std::to_string(model_device[i].second) + ".onnx");
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
        import_weights_from_csv_aMLLibrary(graph, device, path);
        break;
      case Weight_Import_Mode::operation_time:
        import_weights_from_csv_operation_time(graph, device, path);
        break;
      case Weight_Import_Mode::multi_operation_time:
        import_weights_from_csv_multi_operation_time(graph, {device}, path);
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
        import_weights_from_csv_multi_operation_time(graph, devices, path);
        break;
      case Weight_Import_Mode::aMLLibrary:
        import_weights_from_csv_aMLLibrary(graph, devices[index], path);
        break;
      case Weight_Import_Mode::operation_time:
        import_weights_from_csv_operation_time(graph, devices[index], path);
        break;
      default:
        break;
    }
}

std::vector<Parameters>
network_butcher_io::IO_Manager::read_parameters_yaml(std::string const &candidate_resources_path,
                                                     std::string const &candidate_deployments_path,
                                                     std::string const &annotations_path)
{
  std::vector<Parameters> res;

  // Reads the candidate_resources file and tries to construct the network domain hierarchy. Moreover, it will produce
  // the list of avaible resources
  auto [network_domains, subdomain_to_domain, devices_map] = read_candidate_resources(candidate_resources_path);

  // Reads the annotation file
  auto const to_deploy = read_annotations(annotations_path);

  YAML::Node components = YAML::LoadFile(candidate_deployments_path)["Components"];
  for (auto const &model : to_deploy)
    {
      auto const &[model_friendly_name, pair_ram_vram] = model;
      auto const &[model_ram, model_vram]              = pair_ram_vram;

      auto const devices_ram =
        read_candidate_deployments(components, model_friendly_name, model_ram, model_vram, devices_map);

      // If there is more than one "feasible" device
      if (devices_ram.size() > 1)
        {
          // Prepare the collection of parameters for the current set of devices
          for (auto const &devices : get_devices_for_partitions(devices_ram))
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

                      params.bandwidth[{i, j}] =
                        find_bandwidth(network_domains, subdomain_to_domain, first_domain, second_domain);
                    }
                }
            }
        }
    }

  return res;
}

std::vector<std::vector<std::pair<std::string, std::size_t>>>
network_butcher_io::IO_Manager::get_devices_for_partitions(
  const std::vector<std::map<std::string, std::size_t>> &devices_ram)
{
  std::vector<std::vector<std::pair<std::string, std::size_t>>> device_for_partitions;
  device_for_partitions.emplace_back();

  for (std::size_t i = 0; i < devices_ram.size(); ++i)
    {
      auto const size = device_for_partitions.size();
      for (std::size_t k = 0; k < size; ++k)
        {
          auto &partition = device_for_partitions[k];
          if (!devices_ram[i].empty())
            {
              auto const copy = partition;
              auto       it   = devices_ram[i].cbegin();
              partition.push_back({it->first, it->second});
              ++it;
              for (; it != devices_ram[i].cend(); ++it)
                {
                  device_for_partitions.push_back(copy);
                  device_for_partitions.back().push_back({it->first, it->second});
                }
            }
        }
    }

  return device_for_partitions;
}

std::vector<std::map<std::string, std::size_t>>
network_butcher_io::IO_Manager::read_candidate_deployments(YAML::Node const                    &components,
                                                           std::string const                   &model_name,
                                                           std::size_t                          model_ram,
                                                           std::size_t                          model_vram,
                                                           std::map<std::string, device> const &devices_map)
{
  std::vector<std::map<std::string, std::size_t>> devices_ram;

  for (YAML::const_iterator it = components.begin(); it != components.end(); ++it)
    {
      if (it->second["name"] && it->second["name"].as<std::string>().find(model_name) != std::string::npos &&
          it->second["name"].as<std::string>().find("partitionX") != std::string::npos)
        {
          devices_ram.emplace_back();

          // Containers that can be deployed on the different resources
          YAML::Node devices = it->second["Containers"];
          for (YAML::const_iterator device_it = devices.begin(); device_it != devices.end(); ++device_it)
            {
              devices_ram.back()[device_it->second["candidateExecutionResources"][0].as<std::string>()] =
                std::max(device_it->second["memorySize"].as<std::size_t>(), model_ram);
            }
        }
    }

  // Remove the devices with insufficient ram and vram
  for (auto it = devices_ram.begin(); it != devices_ram.end(); ++it)
    {
      std::set<std::string> to_remove;
      for (auto const &[name, ram] : *it)
        {
          auto const &dev = devices_map.find(name)->second;
          if (dev.ram < ram || dev.vram < model_vram)
            to_remove.insert(name);
        }

      for (auto const &name : to_remove)
        {
          it->erase(it->find(name));
        }
    }

  return devices_ram;
}

std::map<std::string, std::pair<std::size_t, std::size_t>>
network_butcher_io::IO_Manager::read_annotations(const std::string &annotations_path)
{
  std::map<std::string, std::pair<std::size_t, std::size_t>> to_deploy;

  YAML::Node annotations = YAML::LoadFile(annotations_path);

  // It will list the different models that have to be partitioned
  for (YAML::const_iterator it = annotations.begin(); it != annotations.end(); ++it)
    {
      if (it->second["partitionable_model"])
        {
          std::cout << it->first << ": " << it->second["component_name"]["name"] << std::endl;

          std::size_t ram  = 0;
          std::size_t vram = 0;

          if (it->second["device_constraints"])
            {
              YAML::Node n = it->second["device_constraints"];

              if (n["ram"])
                ram = n["ram"].as<std::size_t>();
              if (n["vram"])
                vram = n["vram"].as<std::size_t>();
            }

          to_deploy.insert({it->second["component_name"]["name"].as<std::string>(), {ram, vram}});
        }
    }

  return to_deploy;
}


std::pair<bandwidth_type, bandwidth_type>
network_butcher_io::IO_Manager::find_bandwidth(std::map<std::string, network_domain> const &network_domains,
                                               std::map<std::string, std::string> const    &subdomain_to_domain,
                                               std::string                                  first_domain,
                                               std::string                                  second_domain)
{
  auto first_domain_depth  = network_domains.find(first_domain)->second.depth;
  auto second_domain_depth = network_domains.find(second_domain)->second.depth;

  if (second_domain_depth > first_domain_depth)
    {
      std::swap(first_domain_depth, second_domain_depth);
      std::swap(first_domain, second_domain);
    }

  while (first_domain_depth > second_domain_depth)
    {
      if (first_domain == second_domain)
        {
          auto const &dom = network_domains.find(first_domain)->second;
          return std::pair{dom.bandwidth, dom.access_delay};
        }

      --first_domain_depth;
      first_domain = subdomain_to_domain.find(first_domain)->second;
    }

  while (first_domain_depth > 0)
    {
      if (first_domain == second_domain)
        {
          auto const &dom = network_domains.find(first_domain)->second;
          return std::pair{dom.bandwidth, dom.access_delay};
        }

      --first_domain_depth;
      first_domain  = subdomain_to_domain.find(first_domain)->second;
      second_domain = subdomain_to_domain.find(second_domain)->second;
    }

  if (first_domain == second_domain)
    {
      auto const &dom = network_domains.find(first_domain)->second;
      return std::pair{dom.bandwidth, dom.access_delay};
    }
  else
    return std::pair{0, -1.};
}


std::tuple<std::map<std::string, network_domain>, std::map<std::string, std::string>, std::map<std::string, device>>
network_butcher_io::IO_Manager::read_candidate_resources(const std::string &candidate_resources_path)
{
  std::map<std::string, network_domain> network_domains;
  std::map<std::string, std::string>    subdomain_to_domain;
  std::map<std::string, std::size_t>    domain_to_depth;
  std::map<std::string, device>         devices_map;

  auto       resources_file       = YAML::LoadFile(candidate_resources_path);
  YAML::Node network_domains_yaml = resources_file["System"]["NetworkDomains"];

  for (auto const &domain : network_domains_yaml)
    {
      auto const name = domain.first.as<std::string>();

      network_domains[name] = {domain.second["name"].as<std::string>(),
                               domain.second["Bandwidth"].as<std::size_t>(),
                               domain.second["AccessDelay"].as<double>()};

      if (domain_to_depth.find(name) == domain_to_depth.cend())
        domain_to_depth[name] = 0;

      if (domain.second["subNetworkDomains"].size() > 0)
        {
          for (auto const &subdomain : domain.second["subNetworkDomains"])
            {
              auto const subdomain_name = subdomain.as<std::string>();

              subdomain_to_domain[subdomain_name] = name;
              domain_to_depth[subdomain_name]     = domain_to_depth[name] + 1;
            }
        }

      if (domain.second["ComputationalLayers"])
        {
          YAML::Node layers = domain.second["ComputationalLayers"];
          for (YAML::const_iterator layer_it = layers.begin(); layer_it != layers.end(); ++layer_it)
            {
              auto const id = layer_it->second["number"].as<std::size_t>();

              if (layer_it->second["Resources"])
                {
                  YAML::Node resources = layer_it->second["Resources"];
                  for (YAML::const_iterator resource_it = resources.begin(); resource_it != resources.end();
                       ++resource_it)
                    {
                      device dev;

                      dev.name = resource_it->second["name"].as<std::string>();
                      dev.id   = id;

                      dev.ram         = 0;
                      dev.vram        = 0;
                      dev.domain_name = name;

                      if (resource_it->second["memorySize"])
                        {
                          dev.ram = resource_it->second["memorySize"].as<std::size_t>();
                        }

                      if (resource_it->second["accelerators"])
                        {
                          YAML::Node accelerators = resource_it->second["accelerators"];
                          for (YAML::const_iterator it = accelerators.begin(); it != accelerators.end(); ++it)
                            {
                              dev.vram = std::max(dev.vram, it->second["memory"].as<std::size_t>());
                            }
                        }

                      devices_map.insert({resource_it->second["name"].as<std::string>(), dev});
                    }
                }
            }
        }
    }

  for (auto &domain : network_domains)
    {
      domain.second.depth = domain_to_depth[domain.first];
    }

  return {network_domains, subdomain_to_domain, devices_map};
}
