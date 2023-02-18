//
// Created by faccus on 8/7/22.
//
#include "Onnx_importer_helpers.h"

std::unordered_map<std::string, std::vector<network_butcher_types::DynamicType>>
network_butcher_io::Onnx_importer_helpers::process_node_attributes(const onnx::NodeProto &node)
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
          default:
            break;
        }
    }
  return attributes;
}


std::tuple<network_butcher_io::Onnx_importer_helpers::Map_IO, std::set<std::string>, std::set<std::string>>
network_butcher_io::Onnx_importer_helpers::compute_value_infos(
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


  std::vector<std::string> int_res(std::max(tmp_onnx_inputs_ids.size(), initialized.size()));

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
network_butcher_io::Onnx_importer_helpers::get_common_elements(const std::set<std::string>           &onnx_io_ids,
                                                               io_collection_type<type_info_pointer> &io_collection)
{
  std::set<std::string>    in_keys;
  std::vector<std::string> tmp;

  std::transform(io_collection.begin(),
                 io_collection.end(),
                 std::inserter(in_keys, in_keys.end()),
                 [](auto const &pair) { return pair.first; });

  tmp.resize(std::max(onnx_io_ids.size(), in_keys.size()));

  auto it =
    std::set_intersection(onnx_io_ids.cbegin(), onnx_io_ids.cend(), in_keys.cbegin(), in_keys.cend(), tmp.begin());
  tmp.resize(it - tmp.begin());

  return tmp;
}


void
network_butcher_io::Onnx_importer_helpers::populate_id_collection(
  const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
  std::set<std::string>                                            &onnx_io_ids)
{
  std::transform(onnx_io.begin(), onnx_io.end(), std::inserter(onnx_io_ids, onnx_io_ids.end()), [](auto const &el) {
    return el.name();
  });
}


void
network_butcher_io::Onnx_importer_helpers::read_ios(
  network_butcher_io::Onnx_importer_helpers::Map_IO              &input_map,
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
network_butcher_io::Onnx_importer_helpers::read_ios(
  network_butcher_io::Onnx_importer_helpers::Map_IO           &input_map,
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
network_butcher_io::Onnx_importer_helpers::process_node_ios(
  const google::protobuf::RepeatedPtrField<std::basic_string<char>> &io_names,
  io_collection_type<type_info_pointer>                             &parameters_collection,
  Map_IO const                                                      &value_infos,
  std::set<std::string> unused_ios)
{
  io_collection_type<type_info_pointer> res;
  for (auto const &io_name : io_names)
    {
      auto const iterator = value_infos.find(io_name);

      if (iterator != value_infos.cend())
        {
          if (iterator->second->initialized() || (!unused_ios.empty() && unused_ios.find(io_name) != unused_ios.cend()))
            parameters_collection.insert({io_name, iterator->second});
          else
            res.insert({io_name, iterator->second});
        }
    }

  return res;
}

std::set<std::string>
network_butcher_io::Onnx_importer_helpers::find_unused_ios(
  const onnx::GraphProto &onnx_graph)
{
  std::set<std::string> ins_name;
  std::map<std::string, std::set<node_id_type>> ins;

  std::set<std::string> outs_name;
  std::map<std::string, std::set<node_id_type>> outs;

  std::size_t node_id = 0;
  for (auto const &node : onnx_graph.node())
    {
      for (auto const &name : node.input())
        {
          if (std::find_if(onnx_graph.input().cbegin(), onnx_graph.input().cend(), [name](auto const &el) {
                return el.name() == name;
              }) == onnx_graph.input().cend())
            {
              ins_name.insert(name);
              ins[name].insert(node_id);
            }
        }

      for (auto const &name : node.output())
        {
          if (std::find_if(onnx_graph.output().cbegin(), onnx_graph.output().cend(), [name](auto const &el) {
                return el.name() == name;
              }) == onnx_graph.output().cend())
            {
              outs_name.insert(name);
              outs[name].insert(node_id);
            }
        }

      ++node_id;
    }

  std::set<std::string> to_remove_ios;

  {
    std::vector<std::string> intersection(std::min(ins_name.size(), outs_name.size()));
    auto const               it = std::set_intersection(
      ins_name.cbegin(), ins_name.cend(), outs_name.cbegin(), outs_name.cend(), intersection.begin());

    intersection.resize(it - intersection.begin());
    to_remove_ios.insert(intersection.cbegin(), intersection.cend());
  }

  {
    std::set<std::string> union_ios;

    union_ios.insert(ins_name.cbegin(), ins_name.cend());
    union_ios.insert(outs_name.cbegin(), outs_name.cend());

    std::vector<std::string> result(union_ios.size());
    auto const               it = std::set_difference(
      union_ios.cbegin(), union_ios.cend(), to_remove_ios.cbegin(), to_remove_ios.cend(), result.begin());

    to_remove_ios.clear();
    to_remove_ios.insert(result.begin(), it); // Maybe ++it
  }

  return to_remove_ios;
}
