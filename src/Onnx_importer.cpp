//
// Created by faccus on 8/7/22.
//
#include "../include/Onnx_importer.h"

std::unordered_map<std::string, std::vector<network_butcher_types::DynamicType>>
network_butcher_io::Onnx_importer::process_node_attributes(const onnx::NodeProto &node)
{
  auto const help_func =
    [](std::string const                                                                &name,
       auto const                                                                       &array,
       std::unordered_map<std::string, std::vector<network_butcher_types::DynamicType>> &attributes) {
      std::vector<network_butcher_types::DynamicType> add;
      for (auto it = array.cbegin(); it != array.cend(); ++it)
        add.emplace_back(*it);
      attributes.insert({name, add});
    };

  std::unordered_map<std::string, std::vector<network_butcher_types::DynamicType>> attributes;

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
  return attributes;
}


std::tuple<network_butcher_io::Onnx_importer::Map_IO, std::set<std::string>, std::set<std::string>>
network_butcher_io::Onnx_importer::compute_value_infos(
  const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_input,
  const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_output,
  const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_value_info,
  const google::protobuf::RepeatedPtrField<::onnx::TensorProto>    &onnx_initializer)
{
  std::set<std::string> onnx_inputs_ids;
  std::set<std::string> onnx_outputs_ids;

  // Add to onnx_outputs_ids the names of the onnx_outputs
  populate_id_collection(onnx_output, onnx_outputs_ids);

  Map_IO value_infos;


  std::set<std::string> tmp_onnx_inputs_ids;
  populate_id_collection(onnx_input, tmp_onnx_inputs_ids);

  // Collection of the tensor names that have already been initialized
  std::set<std::string> initialized;
  for (auto const &p : onnx_initializer)
    {
      initialized.insert(p.name());
    }


  std::vector<std::string> int_res(std::min(tmp_onnx_inputs_ids.size(), initialized.size()));

  // Add to int_res the non-initialized inputs
  auto it = std::set_difference(tmp_onnx_inputs_ids.cbegin(),
                                tmp_onnx_inputs_ids.cend(),
                                initialized.cbegin(),
                                initialized.cend(),
                                int_res.begin());
  int_res.resize(it - int_res.begin());
  onnx_inputs_ids.insert(int_res.cbegin(), int_res.cend());

  read_ios(value_infos, onnx_input, initialized);
  read_ios(value_infos, onnx_output, initialized);
  read_ios(value_infos, onnx_value_info, initialized);

  read_ios(value_infos, onnx_initializer, initialized);

  return {value_infos, onnx_inputs_ids, onnx_outputs_ids};
}


std::vector<std::string>
network_butcher_io::Onnx_importer::get_common_elements(const std::set<std::string>           &onnx_io_ids,
                                                       io_collection_type<type_info_pointer> &io_collection)
{
  std::set<std::string>    in_keys;
  std::vector<std::string> tmp;

  std::transform(io_collection.begin(),
                 io_collection.end(),
                 std::inserter(in_keys, in_keys.end()),
                 [](auto const &pair) { return pair.first; });

  tmp.resize(std::min(onnx_io_ids.size(), in_keys.size()));

  auto it =
    std::set_intersection(onnx_io_ids.cbegin(), onnx_io_ids.cend(), in_keys.cbegin(), in_keys.cend(), tmp.begin());
  tmp.resize(it - tmp.begin());

  return tmp;
}


void
network_butcher_io::Onnx_importer::populate_id_collection(
  const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
  std::set<std::string>                                            &onnx_io_ids)
{
  std::transform(onnx_io.begin(), onnx_io.end(), std::inserter(onnx_io_ids, onnx_io_ids.end()), [](auto const &el) {
    return el.name();
  });
}


void
network_butcher_io::Onnx_importer::read_ios(network_butcher_io::Onnx_importer::Map_IO                      &input_map,
                                            const google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> &collection,
                                            const std::set<std::string>                                    &initialized)
{
  for (const auto &value_info : collection)
    {
      // Is the onnx::ValueInfoProto well-defined? Is it a tensor that is not in input_map?
      if (value_info.IsInitialized() && value_info.type().has_tensor_type() && !input_map.contains(value_info.name()))
        {
          // Add it to the input_map
          input_map[value_info.name()] =
            std::make_shared<network_butcher_types::Dense_tensor>(value_info, initialized.contains(value_info.name()));
        }
    }
}


void
network_butcher_io::Onnx_importer::read_ios(network_butcher_io::Onnx_importer::Map_IO                   &input_map,
                                            const google::protobuf::RepeatedPtrField<onnx::TensorProto> &collection,
                                            const std::set<std::string>                                 &initialized)
{
  for (const auto &tensor : collection)
    {
      // Is the onnx::TensorProto well-defined? Is it contained in the input_map?
      if (tensor.IsInitialized() && !input_map.contains(tensor.name()))
        {
          // Add it to the input_map
          input_map[tensor.name()] =
            std::make_shared<network_butcher_types::Dense_tensor>(tensor, initialized.contains(tensor.name()));
        }
    }
}


io_collection_type<type_info_pointer>
network_butcher_io::Onnx_importer::process_node_ios(
  const google::protobuf::RepeatedPtrField<std::basic_string<char>> &io_names,
  io_collection_type<type_info_pointer>                             &parameters_collection,
  Map_IO const                                                      &value_infos)
{
  io_collection_type<type_info_pointer> res;
  for (auto const &io_name : io_names)
    {
      auto const iterator = value_infos.find(io_name);

      if (iterator != value_infos.cend())
        {
          if (iterator->second->initialized())
            parameters_collection.insert({io_name, iterator->second});
          else
            res.insert({io_name, iterator->second});
        }
    }

  return res;
}


std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
network_butcher_io::Onnx_importer::import_from_onnx(const std::string &path,
                                                    bool               add_padding_nodes,
                                                    std::size_t        num_devices)
{
  std::map<node_id_type, node_id_type> link_id_nodeproto;

  // Parse from the file the onnx::ModelProto
  onnx::ModelProto onnx_model = network_butcher_utilities::parse_onnx_file(path);

  // Simple renaming
  auto const &onnx_graph       = onnx_model.graph();
  auto const &onnx_input       = onnx_graph.input();
  auto const &onnx_output      = onnx_graph.output();
  auto const &onnx_value_info  = onnx_graph.value_info();
  auto const &onnx_initializer = onnx_graph.initializer();
  auto const &onnx_nodes       = onnx_graph.node();

  // Generate value_infos and the ids (names) of the inputs and outputs
  auto [value_infos, onnx_inputs_ids, onnx_outputs_ids] =
    compute_value_infos(onnx_input, onnx_output, onnx_value_info, onnx_initializer);

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
      auto                                  inputs  = process_node_ios(node.input(), parameters, value_infos);
      auto                                  outputs = process_node_ios(node.output(), parameters, value_infos);

      if (add_padding_nodes)
        {
          if (!get_common_elements(onnx_inputs_ids, inputs).empty())
            inputs[fake_input] = pointer_input;

          if (!get_common_elements(onnx_outputs_ids, outputs).empty())
            outputs[fake_output] = pointer_output;
        }

      auto content = network_butcher_types::Content<type_info_pointer>::make_content(std::move(inputs),
                                                                                     std::move(outputs),
                                                                                     std::move(parameters),
                                                                                     process_node_attributes(node),
                                                                                     std::move(operation_type));
      nodes.emplace_back(std::move(content));

      link_id_nodeproto.emplace(node_id++, onnx_node_id++);
    }

  if (add_padding_nodes)
    {
      io_collection_type<type_info_pointer> tt;
      tt[fake_output] = pointer_output;

      nodes.emplace_back(network_butcher_types::Content<type_info_pointer>(std::move(tt)));
      ++node_id;
    }

  return {network_butcher_types::MWGraph(num_devices, nodes), onnx_model, link_id_nodeproto};
}