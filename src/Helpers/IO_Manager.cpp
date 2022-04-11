//
// Created by faccus on 20/02/22.
//

#include "IO_Manager.h"


std::pair<graph_type, onnx::ModelProto>
IO_Manager::import_from_onnx(const std::string &path, bool add_padding_nodes)
{
  onnx::ModelProto model = utilities::parse_onnx_file(path);
  auto const &onnx_graph = model.graph();

  Map_IO value_infos;

  {
    std::unordered_set<std::string> initialized;
    // Values that are given by the network
    for (auto const &p : onnx_graph.initializer())
      {
        if (p.IsInitialized())
          initialized.insert(p.name());
      }

    onnx_io_read(value_infos, onnx_graph.input(), initialized);
    onnx_io_read(value_infos, onnx_graph.output(), initialized);
    onnx_io_read(value_infos, onnx_graph.value_info(), initialized);
  }

  auto const            &onnx_nodes = onnx_graph.node();
  std::vector<node_type> nodes;
  nodes.reserve(onnx_nodes.size() + 2);

  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt["__fake__input__"] =
        std::make_shared<Dense_tensor>(0, std::vector<shape_type>{});
      nodes.emplace_back(Content({}, tt));
    }

  for (auto const &node : onnx_nodes)
    {
      io_collection_type<type_info_pointer> inputs;
      io_collection_type<type_info_pointer> parameters;
      onnx_process_node(node.input(), inputs, parameters, value_infos);
      /*
            if(inputs.empty())
              continue;*/

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

      Content<type_info_pointer> content(
        inputs, outputs, parameters, attributes, operation_type);

      nodes.emplace_back(std::move(content));
    }

  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt["__fake__output__"] =
        std::make_shared<Dense_tensor>(0, std::vector<shape_type>{});
      nodes.emplace_back(Content(tt, {}));

      (++nodes.begin())
        ->content.input.insert(
          *nodes.front().content.get_output().find("__fake__input__"));
      (++nodes.rbegin())
        ->content.output.insert(
          *nodes.back().content.get_input().find("__fake__output__"));
    }

  return {WGraph(nodes), model};
}

void
IO_Manager::onnx_io_read(
  IO_Manager::Map_IO                                             &input_map,
  const google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> &collection,
  const std::unordered_set<std::string>                          &initialized)
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
}
