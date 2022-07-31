//
// Created by faccus on 20/02/22.
//

#include "../../include/Helpers/IO_Manager.h"


std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
IO_Manager::import_from_onnx(const std::string &path,
                             bool               add_padding_nodes,
                             std::size_t        num_devices)
{
  std::map<node_id_type, node_id_type> link_id_nodeproto;

  onnx::ModelProto onnx_model =
    network_butcher_utilities::parse_onnx_file(path);
  auto const &onnx_graph       = onnx_model.graph();
  auto const &onnx_input       = onnx_graph.input();
  auto const &onnx_output      = onnx_graph.output();
  auto const &onnx_value_info  = onnx_graph.value_info();
  auto const &onnx_initializer = onnx_graph.initializer();

  std::set<std::string> onnx_inputs_ids;
  std::set<std::string> onnx_outputs_ids;

  onnx_populate_id_collection(onnx_output, onnx_outputs_ids);

  Map_IO value_infos;

  {
    std::set<std::string> tmp_onnx_inputs_ids;
    onnx_populate_id_collection(onnx_input, tmp_onnx_inputs_ids);
    std::set<std::string> initialized;
    // Values that are given by the network
    for (auto const &p : onnx_initializer)
      {
        initialized.insert(p.name());
      }


    std::vector<std::string> int_res(
      std::min(tmp_onnx_inputs_ids.size(), initialized.size()));
    auto it = std::set_difference(tmp_onnx_inputs_ids.cbegin(),
                                  tmp_onnx_inputs_ids.cend(),
                                  initialized.cbegin(),
                                  initialized.cend(),
                                  int_res.begin());
    int_res.resize(it - int_res.begin());
    onnx_inputs_ids.insert(int_res.cbegin(), int_res.cend());

    onnx_io_read(value_infos, onnx_input, initialized);
    onnx_io_read(value_infos, onnx_output, initialized);
    onnx_io_read(value_infos, onnx_value_info, initialized);

    onnx_io_read(value_infos, onnx_initializer, initialized);
  }

  auto const            &onnx_nodes = onnx_graph.node();
  std::vector<node_type> nodes;
  nodes.reserve(onnx_nodes.size() + 2);

  auto const pointer_output =
    std::make_shared<network_butcher_types::Dense_tensor>(
      0, std::vector<shape_type>{});
  auto const pointer_input =
    std::make_shared<network_butcher_types::Dense_tensor>(
      0, std::vector<shape_type>{});

  auto const fake_output = "__fake__output__";
  auto const fake_input  = "__fake__input__";

  node_id_type node_id      = 0;
  node_id_type onnx_node_id = 0;

  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt[fake_input] = pointer_input;

      nodes.emplace_back(network_butcher_types::Content({}, std::move(tt)));
      ++node_id;
    }

  auto const help_func =
    [](std::string const &name,
       auto const        &array,
       std::unordered_map<std::string,
                          std::vector<network_butcher_types::DynamicType>>
         &attributes) {
      std::vector<network_butcher_types::DynamicType> add;
      for (auto it = array.cbegin(); it != array.cend(); ++it)
        add.emplace_back(*it);
      attributes.insert({name, add});
    };

  for (auto const &node : onnx_nodes)
    {
      auto operation_type = node.op_type();

      io_collection_type<type_info_pointer> inputs;
      io_collection_type<type_info_pointer> parameters;
      onnx_process_node(node.input(), inputs, parameters, value_infos);

      io_collection_type<type_info_pointer> outputs;
      onnx_process_node(node.output(), outputs, parameters, value_infos);

      std::unordered_map<std::string,
                         std::vector<network_butcher_types::DynamicType>>
        attributes;

      for (auto const &attribute : node.attribute())
        {
          switch (attribute.type())
            {
                case onnx::AttributeProto_AttributeType_FLOAT: {
                  attributes.insert({attribute.name(), {attribute.f()}});
                  break;
                }
                case onnx::AttributeProto_AttributeType_FLOATS: {
                  help_func(attribute.name(), attribute.floats(), attributes);
                  break;
                }
                case onnx::AttributeProto_AttributeType_INT: {
                  attributes.insert({attribute.name(), {attribute.i()}});
                  break;
                }
                case onnx::AttributeProto_AttributeType_INTS: {
                  help_func(attribute.name(), attribute.ints(), attributes);
                  break;
                }
                case onnx::AttributeProto_AttributeType_STRING: {
                  attributes.insert({attribute.name(), {attribute.s()}});
                }
                case onnx::AttributeProto_AttributeType_STRINGS: {
                  help_func(attribute.name(), attribute.strings(), attributes);
                  break;
                }
            }
        }

      std::transform(operation_type.begin(),
                     operation_type.end(),
                     operation_type.begin(),
                     ::tolower);

      if (add_padding_nodes)
        {
          if (!get_common_elements(onnx_inputs_ids, inputs).empty())
            inputs[fake_input] = pointer_input;

          if (!get_common_elements(onnx_outputs_ids, outputs).empty())
            outputs[fake_output] = pointer_output;
        }

      network_butcher_types::Content<type_info_pointer> content(
        inputs, outputs, parameters, attributes, operation_type);
      nodes.emplace_back(std::move(content));

      link_id_nodeproto.emplace(node_id++, onnx_node_id++);
    }

  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt[fake_output] = pointer_output;

      nodes.emplace_back(network_butcher_types::Content(std::move(tt), {}));
      ++node_id;
    }

  return {network_butcher_types::MWGraph(num_devices, nodes),
          onnx_model,
          link_id_nodeproto};
}


std::vector<std::string>
IO_Manager::get_common_elements(
  const std::set<std::string>           &onnx_io_ids,
  io_collection_type<type_info_pointer> &io_collection)
{
  std::set<std::string>    in_keys;
  std::vector<std::string> tmp;

  std::transform(io_collection.begin(),
                 io_collection.end(),
                 std::inserter(in_keys, in_keys.end()),
                 [](auto const &pair) { return pair.first; });

  tmp.resize(std::min(onnx_io_ids.size(), in_keys.size()));

  auto it = std::set_intersection(onnx_io_ids.cbegin(),
                                  onnx_io_ids.cend(),
                                  in_keys.cbegin(),
                                  in_keys.cend(),
                                  tmp.begin());
  tmp.resize(it - tmp.begin());
  return tmp;
}


void
IO_Manager::onnx_populate_id_collection(
  const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
  std::set<std::string>                                            &onnx_io_ids)
{
  std::transform(onnx_io.begin(),
                 onnx_io.end(),
                 std::inserter(onnx_io_ids, onnx_io_ids.end()),
                 [](auto const &el) { return el.name(); });
}


void
IO_Manager::onnx_io_read(
  IO_Manager::Map_IO                                             &input_map,
  const google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> &collection,
  const std::set<std::string>                                    &initialized)
{
  for (const auto &param : collection)
    {
      if (param.IsInitialized())
        {
          const auto &type = param.type();

          if (type.has_tensor_type() && !input_map.contains(param.name()))
            {
              input_map[param.name()] =
                std::make_shared<network_butcher_types::Dense_tensor>(
                  param, initialized.contains(param.name()));
            }
        }
    }
}


void
IO_Manager::onnx_io_read(
  IO_Manager::Map_IO                                          &input_map,
  const google::protobuf::RepeatedPtrField<onnx::TensorProto> &collection,
  const std::set<std::string>                                 &initialized)
{
  for (const auto &param : collection)
    {
      if (param.IsInitialized() &&
          input_map.find(param.name()) == input_map.cend())
        {
          input_map[param.name()] =
            std::make_shared<network_butcher_types::Dense_tensor>(
              param, initialized.contains(param.name()));
        }
    }
}


void
IO_Manager::onnx_process_node(
  const google::protobuf::RepeatedPtrField<std::basic_string<char>> &io_names,
  io_collection_type<type_info_pointer> &io_collection,
  io_collection_type<type_info_pointer> &parameters_collection,
  Map_IO const                          &value_infos)
{
  for (auto const &io_name : io_names)
    {
      auto const iterator = value_infos.find(io_name);

      if (iterator != value_infos.cend())
        {
          if (iterator->second->initialized())
            parameters_collection.insert({io_name, iterator->second});
          else
            io_collection.insert({io_name, iterator->second});
        }
    }
  {}
}


void
IO_Manager::export_to_onnx(const onnx::ModelProto &model, std::string path)
{
  network_butcher_utilities::output_onnx_file(model, path);
}


void
IO_Manager::export_network_infos_to_csv(graph_type const       &graph,
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

          auto       ins  = content.get_input();
          auto       outs = content.get_output();
          auto const kernel_iterator =
            content.get_attributes().find("kernel_shape");

          if (kernel_iterator != content.get_attributes().cend())
            {
              auto out_it = outs.cbegin();
              while (out_it != outs.cend() &&
                     out_it->first == "__fake__output__")
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

              file_out << node.get_id() << "," << H_f << "," << W_f << ","
                       << C_in << "," << C_out << "," << flops << ",0"
                       << std::endl;
            }
        }
    }

  file_out.close();
}


void
IO_Manager::import_weights_from_csv_aMLLibrary(graph_type        &graph,
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
               it->content.get_attributes().find("kernel_shape") !=
                 it->content.get_attributes().cend()))
        {
          ++it;
        }

      if (it == graph.get_nodes().cend())
        return;

      for (auto const &successor :
           graph.get_dependencies()[it->get_id()].second)
        {
          graph.set_weigth(device, {it->get_id(), successor}, tmp_weight);
        }

      ++it;
    }

  file_in.close();
}


void
IO_Manager::import_weights_from_csv_operation_time(graph_type        &graph,
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
             network_butcher_utilities::trim_copy(
               network_butcher_utilities::to_lowercase_copy(operation_name)) !=
               network_butcher_utilities::trim_copy(
                 network_butcher_utilities::to_lowercase_copy(
                   it->content.get_operation_id())))
        {
          ++it;
        }

      if (it == graph.get_nodes().cend())
        return;

      for (auto const &successor :
           graph.get_dependencies()[it->get_id()].second)
        {
          graph.set_weigth(device, {it->get_id(), successor}, tmp_weight);
        }

      ++it;
    }

  file_in.close();
}


void
IO_Manager::import_weights_from_csv_multi_operation_time(
  graph_type              &graph,
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
             network_butcher_utilities::to_lowercase_copy(
               it->content.get_operation_id()) !=
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

          for (auto const &successor :
               graph.get_dependencies()[it->get_id()].second)
            {
              graph.set_weigth(device[j],
                               {it->get_id(), successor},
                               tmp_weight);
            }

          ++it;
          ++j;
        }
    }

  file_in.close();
}


Parameters
IO_Manager::read_parameters(const std::string &path)
{
  GetPot file(path);

  Parameters        res;
  std::string const basic_infos = "basic_config";

  res.model_name       = file(basic_infos + "/model_name", "model");
  res.model_path       = file(basic_infos + "/model_path", "");
  res.export_directory = file(basic_infos + "/export_directory", "ksp_result");


  res.K                    = file(basic_infos + "/K", 100);
  std::string const method = network_butcher_utilities::trim_copy(
    network_butcher_utilities::to_lowercase_copy(
      file(basic_infos + "/method", "")));

  if (method == "Eppstein")
    res.method = KSP_Method::Eppstein;
  else
    res.method = KSP_Method::Lazy_Eppstein;

  res.backward_connections_allowed =
    file(basic_infos + "/backward_connections_allowed", false);

  std::string const weight_import_method = network_butcher_utilities::trim_copy(
    network_butcher_utilities::to_lowercase_copy(
      file(basic_infos + "/weight_import_mode", "aMLLibrary")));

  if (weight_import_method == "amllibrary")
    res.weight_import_mode = Weight_Import_Mode::aMLLibrary;
  else if (weight_import_method == "multi_operation_time")
    res.weight_import_mode = Weight_Import_Mode::multi_operation_time;
  else
    res.weight_import_mode = Weight_Import_Mode::operation_time;

  res.memory_constraint = file(basic_infos + "/memory_constraint", false);
  if (res.memory_constraint)
    {
      std::string const memory_constraint_type =
        network_butcher_utilities::trim_copy(
          network_butcher_utilities::to_lowercase_copy(
            file(basic_infos + "/memory_constraint_type", "none")));

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
          res.memory_constraint_type =
            Memory_Constraint_Type::Preload_Parameters;
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
  for (std::size_t i = 0; i < num_devices; ++i)
    {
      for (std::size_t j = i + 1; j < num_devices; ++j)
        {
          res.bandwidth[{i, j}] = file(bndw + "/from_" + std::to_string(i) +
                                         "_to_" + std::to_string(j),
                                       .0);
        }
    }

  if (res.backward_connections_allowed)
    {
      for (std::size_t i = num_devices; i >= 0; --i)
        {
          for (std::size_t j = i - 1; j >= 0; --j)
            {
              res.bandwidth[{i, j}] = file(bndw + "/from_" + std::to_string(i) +
                                             "_to_" + std::to_string(j),
                                           .0);
            }
        }
    }

  return res;
}


void
IO_Manager::export_network_partitions(
  const Parameters                                 &params,
  graph_type const                                 &graph,
  onnx::ModelProto const                           &model,
  std::map<node_id_type, node_id_type> const       &link_id_nodeproto,
  const network_butcher_types::Weighted_Real_Paths &paths)
{
  network_butcher_utilities::create_directory(params.export_directory);
  for (std::size_t j = 0; j < paths.size(); ++j)
    {
      network_butcher_utilities::create_directory(params.export_directory +
                                                  "/" + std::to_string(j));

      auto const model_device =
        reconstruct_model(paths[j].second, model, graph, link_id_nodeproto);

      for (std::size_t i = 0; i < model_device.size(); ++i)
        export_to_onnx(model_device[i].first,
                       params.export_directory + "/" + std::to_string(j) + "/" +
                         params.model_name + "-" + std::to_string(i) +
                         "-device-" + std::to_string(model_device[i].second) +
                         ".onnx");
    }
}


void
IO_Manager::import_weights(Weight_Import_Mode const &weight_mode,
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
IO_Manager::import_weights(Weight_Import_Mode const &weight_mode,
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