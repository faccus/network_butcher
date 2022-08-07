//
// Created by faccus on 20/02/22.
//

#include "../include/IO_Manager.h"


std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
network_butcher_io::IO_Manager::import_from_onnx(const std::string &path,
                                                 bool               add_padding_nodes,
                                                 std::size_t        num_devices)
{
  return Onnx_importer::import_from_onnx(path, add_padding_nodes, num_devices);
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
