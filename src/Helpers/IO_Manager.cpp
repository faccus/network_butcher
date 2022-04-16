//
// Created by faccus on 20/02/22.
//

#include "IO_Manager.h"


std::pair<graph_type, onnx::ModelProto>
IO_Manager::import_from_onnx(const std::string &path, bool add_padding_nodes)
{
  onnx::ModelProto model = utilities::parse_onnx_file(path);
  auto const &onnx_graph = model.graph();
  auto const &onnx_input = onnx_graph.input();
  auto const &onnx_output = onnx_graph.output();

  std::set<std::string> onnx_inputs_ids;
  std::set<std::string> onnx_outputs_ids;

  onnx_populate_id_collection(onnx_output, onnx_outputs_ids);

  Map_IO value_infos;

  {
    std::set<std::string> tmp_onnx_inputs_ids;
    onnx_populate_id_collection(onnx_input, tmp_onnx_inputs_ids);
    std::set<std::string> initialized;
    // Values that are given by the network
    for (auto const &p : onnx_graph.initializer())
      {
        if (p.IsInitialized())
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

    onnx_io_read(value_infos, onnx_graph.input(), initialized);
    onnx_io_read(value_infos, onnx_graph.output(), initialized);
    onnx_io_read(value_infos, onnx_graph.value_info(), initialized);
  }

  auto const            &onnx_nodes = onnx_graph.node();
  std::vector<node_type> nodes;
  nodes.reserve(onnx_nodes.size() + 2);

  auto const                            pointer_output =
    std::make_shared<Dense_tensor>(0, std::vector<shape_type>{});
  auto const                            pointer_intput =
    std::make_shared<Dense_tensor>(0, std::vector<shape_type>{});

  auto const fake_output = "__fake__output__";
  auto const fake_input = "__fake__input__";

  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt[fake_input] = pointer_intput;

      nodes.emplace_back(Content({}, std::move(tt)));
    }

  for (auto const &node : onnx_nodes)
    {
      io_collection_type<type_info_pointer> inputs;
      io_collection_type<type_info_pointer> parameters;
      onnx_process_node(node.input(), inputs, parameters, value_infos);

      io_collection_type<type_info_pointer> outputs;
      onnx_process_node(node.output(), outputs, parameters, value_infos);


      std::unordered_map<std::string, std::vector<std::size_t>> attributes;

      for (auto const &attribute : node.attribute())
        {
          if (attribute.name() == "kernel_shape")
            {
              std::vector<std::size_t> add;
              for (auto it = attribute.ints().begin();
                   it != attribute.ints().end();
                   ++it)
                add.push_back(*it);
              attributes.insert({attribute.name(), add});
            }
        }

      std::string operation_type = node.op_type();
      std::transform(operation_type.begin(),
                     operation_type.end(),
                     operation_type.begin(),
                     ::tolower);

      if (!get_common_elements(onnx_inputs_ids, inputs).empty())
        inputs[fake_input] = pointer_intput;

      if (!get_common_elements(onnx_outputs_ids, outputs).empty())
        outputs[fake_output] = pointer_output;

      Content<type_info_pointer> content(
        inputs, outputs, parameters, attributes, operation_type);
      nodes.emplace_back(std::move(content));
    }

  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt[fake_output] = pointer_output;

      nodes.emplace_back(Content(std::move(tt), {}));
    }

  return {WGraph(nodes), model};
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
              input_map[param.name()] = std::make_shared<Dense_tensor>(
                param, initialized.contains(param.name()));
            }
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

      if(iterator != value_infos.cend())
        {
          if(iterator->second->initialized())
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
  utilities::output_onnx_file(model, path);
}

void
IO_Manager::regression_parameters_to_excel(
  std::pair<graph_type, onnx::ModelProto> const &input)
{
  auto const &graph = input.first;
  auto const &model = input.second;

  std::fstream file_out;
  file_out.open("butcher_predict.csv", std::ios_base::out);

  file_out << "Layer,Hf,Wf,Cin,Cout,FLOPS,Time" << std::endl;

  for (auto const &node : graph.get_nodes())
    {
      if (node.content.operation_id == "conv")
        {
          auto const &content = node.content;

          auto const &ins  = content.get_input();
          auto const &outs = content.get_output();
          auto const  kernel_iterator =
            content.get_attributes().find("kernel_shape");

          if (ins.size() == 1 && outs.size() == 1 &&
              kernel_iterator != content.get_attributes().cend())
            {
              auto const &out_shape    = outs.begin()->second->get_shape();
              auto const &in_shape     = ins.begin()->second->get_shape();
              auto const &kernel_shape = kernel_iterator->second;

              auto const &H_f = kernel_iterator->second[0];
              auto const &W_f = kernel_iterator->second[1];


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
}