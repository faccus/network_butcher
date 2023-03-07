//
// Created by faccus on 20/02/22.
//

#include "IO_Manager.h"


std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
network_butcher_io::IO_Manager::import_from_onnx(std::string const &path,
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

      auto content =
        network_butcher_types::Content<type_info_pointer>::make_content(std::move(inputs),
                                                                        std::move(outputs),
                                                                        std::move(parameters),
                                                                        Onnx_importer_helpers::process_node_attributes(
                                                                          node),
                                                                        std::move(operation_type));
      nodes.emplace_back(std::move(content));
      //nodes.back().name = Utilities::to_lowercase_copy(node.op_type() + "_" + std::to_string(onnx_node_id));
      nodes.back().name = node.name();

      link_id_nodeproto.emplace(node_id++, onnx_node_id++);
    }

  // If add_output_padding, then we will add a "fake" output node
  if (add_output_padding)
    {
      io_collection_type<type_info_pointer> tt;
      tt["__fake__output__"] = pointer_output;

      nodes.emplace_back(network_butcher_types::Content<type_info_pointer>(std::move(tt)));
      ++node_id;
    }


  return {network_butcher_types::MWGraph(num_devices, nodes), onnx_model, link_id_nodeproto};
}


void
network_butcher_io::IO_Manager::export_to_onnx(const onnx::ModelProto &model, std::string path)
{
  Utilities::output_onnx_file(model, path);
}


void
network_butcher_io::IO_Manager::old_export_network_infos_to_csv(graph_type const       &graph,
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
network_butcher_io::IO_Manager::export_network_infos_to_csv(graph_type const &graph, std::string const &path)
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
network_butcher_io::IO_Manager::utilities::import_weights_aMLLibrary(graph_type        &graph,
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

      for (auto const &successor : graph.get_neighbors()[it->get_id()].second)
        {
          graph.set_weight(device, {it->get_id(), successor}, tmp_weight);
        }

      ++it;
    }

  file_in.close();
}


void
network_butcher_io::IO_Manager::utilities::import_weights_custom_csv_operation_time(graph_type        &graph,
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
network_butcher_io::IO_Manager::utilities::csv_assembler(const std::vector<std::vector<std::string>> &content,
                                                         const std::string                           &path)
{
  std::fstream file_out;
  file_out.open(path, std::ios_base::out);

  for(auto const &row : content) {
      for(std::size_t i = 0; i < row.size(); ++i) {
          file_out << row[i];
          if(i != row.size() - 1)
            file_out << ",";
        }
      file_out << std::endl;
    }

  file_out.close();
}


std::string
network_butcher_io::IO_Manager::utilities::aMLLibrary_generate_csv_entry(
  const std::string                            &entry,
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
    return aMLLibrary_generate_csv_entry(entry, {}, node, params);
}


std::string
network_butcher_io::IO_Manager::utilities::aMLLibrary_generate_csv_entry(
  const std::string                            &entry,
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
                            basic_info.memory * 8 * (params.bandwidth.cbegin()->second.first * std::pow(10, 6));
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
network_butcher_io::IO_Manager::utilities::import_weights_aMLLibrary_local(
  graph_type                                   &graph,
  const network_butcher_parameters::Parameters &params)
{
#if PYBIND_ACTIVE

  if(params.devices.size() == 2 && !params.backward_connections_allowed) {
      pybind11::initialize_interpreter();

      auto const macs = IO_Manager::read_network_info_onnx_tool(IO_Manager::network_info_onnx_tool(params));
      auto const csv_path = "aMLLibrary_input.csv";

      if(!Utilities::directory_exists(params.temporary_directory))
        Utilities::create_directory(params.temporary_directory);

      if(Utilities::file_exists(csv_path))
        Utilities::file_delete(csv_path);

      auto const initial_row = Utilities::trim_copy(params.weight_csv_features);

      std::vector<std::vector<std::string>> aMLLibrary_input;
      aMLLibrary_input.push_back(initial_row);
      aMLLibrary_input.front().emplace_back("1stInfTime");
      aMLLibrary_input.front().emplace_back("2ndInfTime");

      for (auto const &node : graph.get_nodes())
        {
          auto                     info_it = macs.find(node.name);
          std::vector<std::string> row;
          row.reserve(aMLLibrary_input.front().size());

          if (info_it != macs.cend())
            {
              for (auto const &entry : aMLLibrary_input.front())
                {
                  row.emplace_back(aMLLibrary_generate_csv_entry(entry, info_it->second, node, params));
                }
            }
          else
            {
              for (auto const &entry : aMLLibrary_input.front())
                {
                  row.emplace_back(aMLLibrary_generate_csv_entry(entry, node, params));
                }
            }

          aMLLibrary_input.push_back(std::move(row));
        }

      csv_assembler(aMLLibrary_input, csv_path);

      add_aMLLibrary_to_path();

      execute_weight_generator(params.devices.front().weights_path,
                               "predict_0.ini",
                               Utilities::combine_path(params.temporary_directory, "predict_0"),
                               params.package_aMLLibrary_location);
      execute_weight_generator(params.devices.back().weights_path,
                               "predict_1.ini",
                               Utilities::combine_path(params.temporary_directory, "predict_1"),
                               params.package_aMLLibrary_location);

      pybind11::finalize_interpreter();
    }
  else {
      std::cout << "aMLLibrary_local works only with two devices!" << std::endl;
    }

#else
  std::cout << "PyBind should be turned on in order to produce locally the weights!" << std::endl;
#endif
}

void
network_butcher_io::IO_Manager::utilities::import_weights_official_csv_multi_operation_time(
  graph_type              &graph,
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
network_butcher_io::IO_Manager::utilities::import_weights_custom_csv_multi_operation_time(
  graph_type              &graph,
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


network_butcher_parameters::Parameters
network_butcher_io::IO_Manager::read_parameters(const std::string &path)
{
  GetPot file(path);

  network_butcher_parameters::Parameters res;
  std::string const                      basic_infos = "basic_config";

  res.model_name       = file(basic_infos + "/model_name", "model");
  res.model_path       = file(basic_infos + "/model_path", "");
  res.export_directory = file(basic_infos + "/export_directory", "ksp_result");
  res.temporary_directory = file(basic_infos + "/temporary_directory", "tmp");
  res.package_onnx_tool_location = file(basic_infos + "/package_onnx_tool_location", "");
  res.package_aMLLibrary_location = file(basic_infos + "/package_aMLLibrary_location", "");

  //package_aMLLibrary_location = /home/faccus/CLionProjects/network_butcher/aMLLibrary

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
    Utilities::trim_copy(Utilities::to_lowercase_copy(file(basic_infos + "/weight_import_mode", "aMLLibrary")));

  if (weight_import_method == "amllibrary")
    res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::aMLLibrary_direct_read;
  else if (weight_import_method == "multi_operation_time")
    res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::multi_operation_time;
  else if (weight_import_method == "official_operation_time")
    res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::official_operation_time;
  else if (weight_import_method == "local_generation")
      res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::aMLLibrary_local_inference;
  else
    res.weight_import_mode = network_butcher_parameters::Weight_Import_Mode::operation_time;

  auto const len = file.vector_variable_size(basic_infos + "/weight_csv_features");
  res.weight_csv_features.reserve(len);
  for(std::size_t i = 0; i < len; ++i)
    res.weight_csv_features.emplace_back(file(basic_infos + "/weight_csv_features", i, ""));

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
network_butcher_io::IO_Manager::export_network_partitions(const network_butcher_parameters::Parameters &params,
                                                          onnx::ModelProto const                       &model,
                                                          std::map<node_id_type, node_id_type> const &link_id_nodeproto,
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
network_butcher_io::IO_Manager::import_weights(network_butcher_parameters::Weight_Import_Mode const &weight_mode,
                                               graph_type                                           &graph,
                                               std::string const                                    &path,
                                               std::size_t                                           device)
{
  switch (weight_mode)
    {
      case network_butcher_parameters::Weight_Import_Mode::aMLLibrary_direct_read:
        utilities::import_weights_aMLLibrary(graph, device, path);
        break;
      case network_butcher_parameters::Weight_Import_Mode::operation_time:
        utilities::import_weights_custom_csv_operation_time(graph, device, path);
        break;
      case network_butcher_parameters::Weight_Import_Mode::multi_operation_time:
        utilities::import_weights_custom_csv_multi_operation_time(graph, {device}, path);
        break;
      case network_butcher_parameters::Weight_Import_Mode::official_operation_time:
        utilities::import_weights_official_csv_multi_operation_time(graph, {device}, path);
        break;
      default:
        std::cout << "The specified Weight_Import_Mode is either not avaible or not found. Please, check that you "
                     "specified the correct import mode!"
                  << std::endl;
        break;
    }
}


void
network_butcher_io::IO_Manager::import_weights(network_butcher_parameters::Weight_Import_Mode const &weight_mode,
                                               graph_type                                           &graph,
                                               std::string const                                    &path,
                                               std::vector<std::size_t> const                       &devices)
{
  switch (weight_mode)
    {
      case network_butcher_parameters::Weight_Import_Mode::multi_operation_time:
        utilities::import_weights_custom_csv_multi_operation_time(graph, devices, path);
        break;
      case network_butcher_parameters::Weight_Import_Mode::official_operation_time:
        utilities::import_weights_official_csv_multi_operation_time(graph, devices, path);
        break;
      default:
        std::cout << "The specified Weight_Import_Mode is either not avaible or not found. Please, check that you "
                     "specified the correct import mode!"
                  << std::endl;
        break;
    }
}


void
network_butcher_io::IO_Manager::utilities::reconstruct_model_and_export(
  const network_butcher_types::Real_Path     &partitions,
  const onnx::ModelProto                     &original_model,
  const std::map<node_id_type, node_id_type> &link_id_nodeproto,
  std::unordered_map<std::string,
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


std::pair<bool, onnx::ModelProto>
network_butcher_io::IO_Manager::reconstruct_model_from_partition(
  const network_butcher_types::Real_Partition &partition,
  const onnx::ModelProto                      &original_model,
  const std::map<node_id_type, node_id_type>  &link_id_nodeproto,
  const std::unordered_map<std::string,
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


#if YAML_CPP_ACTIVE

std::vector<network_butcher_parameters::Parameters>
network_butcher_io::IO_Manager::read_parameters_yaml(std::string const &candidate_resources_path,
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
#endif

#if PYBIND_ACTIVE

void network_butcher_io::IO_Manager::utilities::add_aMLLibrary_to_path() {
  using namespace pybind11::literals;
  namespace py = pybind11;

  py::object path     = py::module_::import("sys").attr("path");
  py::object inserter = path.attr("append");
  inserter(NN_Source_Path);
}


void
network_butcher_io::IO_Manager::utilities::execute_weight_generator(const std::string &regressor_file,
                                                                    const std::string &config_file,
                                                                    const std::string &output_path,
                                                                    const std::string &package_path)
{
  using namespace pybind11::literals;
  namespace py = pybind11;

  if(Utilities::directory_exists(output_path))
    Utilities::directory_delete(output_path);

  py::object Predictor = py::module_::import("aMLLibrary.model_building.predictor").attr("Predictor");
  py::object predict =
    Predictor("regressor_file"_a = regressor_file, "output_folder"_a = output_path, "debug"_a = false).attr("predict");

  predict("config_file"_a=config_file, "mape_to_file"_a=false);
}


std::string
network_butcher_io::IO_Manager::network_info_onnx_tool(const std::string &model_path,
                                                       const std::string &package_onnx_tool_location,
                                                       const std::string &temporary_directory)
{
  using namespace pybind11::literals;
  namespace py = pybind11;

  if(!package_onnx_tool_location.empty()) {

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
network_butcher_io::IO_Manager::network_info_onnx_tool(const network_butcher_parameters::Parameters &params)
{
  return network_info_onnx_tool(params.model_path, params.package_onnx_tool_location, params.temporary_directory);
}

std::map<std::string, network_butcher_io::IO_Manager::utilities::onnx_tool_output>
network_butcher_io::IO_Manager::read_network_info_onnx_tool(const std::string &path)
{
  std::map<std::string, network_butcher_io::IO_Manager::utilities::onnx_tool_output> res;

  // Import the file
  std::fstream file_in;
  file_in.open(path, std::ios_base::in);
  weight_type tmp_weight;

  std::string tmp_line;
  std::getline(file_in, tmp_line);

  // If not the end of file,
  while (!file_in.eof())
    {
      std::string operation;
      IO_Manager::utilities::onnx_tool_output info;

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