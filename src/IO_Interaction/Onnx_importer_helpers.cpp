//
// Created by faccus on 8/7/22.
//
#include "Onnx_importer_helpers.h"

namespace network_butcher::io
{
  std::unordered_map<std::string, network_butcher::types::DynamicType>
  Onnx_importer_helpers::process_node_attributes(onnx::NodeProto const &node)
  {
    using DynamicType = network_butcher::types::DynamicType;
    using namespace Utilities;

    std::unordered_map<std::string, DynamicType> attributes;

    for (auto const &attribute : node.attribute())
      {
        switch (attribute.type())
          {
              case onnx::AttributeProto_AttributeType_FLOAT: {
                attributes.emplace(attribute.name(), std::vector<float>{attribute.f()});
                break;
              }
              case onnx::AttributeProto_AttributeType_FLOATS: {
                attributes.emplace(attribute.name(), converter(attribute.floats()));
                break;
              }
              case onnx::AttributeProto_AttributeType_INT: {
                attributes.emplace(attribute.name(), std::vector<long int>{attribute.i()});
                break;
              }
              case onnx::AttributeProto_AttributeType_INTS: {
                attributes.emplace(attribute.name(), converter(attribute.ints()));
                break;
              }
              case onnx::AttributeProto_AttributeType_STRING: {
                attributes.emplace(attribute.name(), std::vector<std::string>{attribute.s()});
              }
              case onnx::AttributeProto_AttributeType_STRINGS: {
                attributes.emplace(attribute.name(), converter(attribute.strings()));
                break;
              }
            default:
              break;
          }
      }

    return attributes;
  }


  std::tuple<Onnx_importer_helpers::Map_IO, std::set<std::string>, std::set<std::string>>
  Onnx_importer_helpers::compute_value_infos(
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
    auto it = std::set_difference(PAR,
                                  tmp_onnx_inputs_ids.cbegin(),
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
  Onnx_importer_helpers::get_common_elements(const std::set<std::string>           &onnx_io_ids,
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
  Onnx_importer_helpers::populate_id_collection(
    const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
    std::set<std::string>                                            &onnx_io_ids)
  {
    std::transform(onnx_io.begin(), onnx_io.end(), std::inserter(onnx_io_ids, onnx_io_ids.end()), [](auto const &el) {
      return el.name();
    });
  }


  void
  Onnx_importer_helpers::read_ios(Onnx_importer_helpers::Map_IO                                  &input_map,
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
              std::make_shared<network_butcher::types::Dense_tensor>(value_info,
                                                                     initialized.contains(value_info.name()));
          }
      }
  }


  void
  Onnx_importer_helpers::read_ios(Onnx_importer_helpers::Map_IO                               &input_map,
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
              std::make_shared<network_butcher::types::Dense_tensor>(tensor, initialized.contains(tensor.name()));
          }
      }
  }


  io_collection_type<type_info_pointer>
  Onnx_importer_helpers::process_node_ios(const google::protobuf::RepeatedPtrField<std::basic_string<char>> &io_names,
                                          io_collection_type<type_info_pointer> &parameters_collection,
                                          Map_IO const                          &value_infos,
                                          std::set<std::string> const           &unused_ios)
  {
    io_collection_type<type_info_pointer> res;
    for (auto const &io_name : io_names)
      {
        auto const iterator = value_infos.find(io_name);

        if (iterator != value_infos.cend())
          {
            if (iterator->second->initialized() ||
                (!unused_ios.empty() && unused_ios.find(io_name) != unused_ios.cend()))
              parameters_collection.insert({io_name, iterator->second});
            else
              res.insert({io_name, iterator->second});
          }
      }

    return res;
  }


  std::set<std::string>
  Onnx_importer_helpers::find_unused_ios(const onnx::GraphProto &onnx_graph)
  {
    std::set<std::string>                         ins_name;
    std::map<std::string, std::set<node_id_type>> ins;

    std::set<std::string>                         outs_name;
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
  Onnx_importer_helpers::prepared_import_onnx
  Onnx_importer_helpers::prepare_import_from_onnx(const onnx::GraphProto &onnx_graph,
                                                  bool                    add_input_padding,
                                                  bool                    add_output_padding,
                                                  bool                    unused_ios)
  {
    prepared_import_onnx res;

    auto const &[value_infos, onnx_inputs_ids, onnx_outputs_ids] =
      compute_value_infos(onnx_graph.input(), onnx_graph.output(), onnx_graph.value_info(), onnx_graph.initializer());

    res.value_infos      = value_infos;
    res.onnx_inputs_ids  = onnx_inputs_ids;
    res.onnx_outputs_ids = onnx_outputs_ids;

    res.pointer_output = std::make_shared<network_butcher::types::Dense_tensor>(0, std::vector<shape_type>{});
    res.pointer_input  = std::make_shared<network_butcher::types::Dense_tensor>(0, std::vector<shape_type>{});

    if (unused_ios)
      res.unused_ios_set = find_unused_ios(onnx_graph);

    res.add_input_padding  = add_input_padding;
    res.add_output_padding = add_output_padding;

    return res;
  }
  node_type
  Onnx_importer_helpers::process_node(const onnx::NodeProto                             &node,
                                      const Onnx_importer_helpers::prepared_import_onnx &prepared_data)
  {
    auto const &value_infos    = prepared_data.value_infos;
    auto const &unused_ios_set = prepared_data.unused_ios_set;

    auto operation_type = network_butcher::Utilities::to_lowercase_copy(node.op_type());

    io_collection_type<type_info_pointer> parameters;
    auto inputs  = process_node_ios(node.input(), parameters, value_infos, unused_ios_set);
    auto outputs = process_node_ios(node.output(), parameters, value_infos, unused_ios_set);

    // If the inputs of the node are the inputs of the NN, then add the connection with the padding
    // node
    if (prepared_data.add_input_padding && !get_common_elements(prepared_data.onnx_inputs_ids, inputs).empty())
      {
        inputs["__fake__input__"] = prepared_data.pointer_input;
      }

    // If the inputs of the node are the outputs of the NN, then add the connection with the padding
    // node
    if (prepared_data.add_output_padding && !get_common_elements(prepared_data.onnx_outputs_ids, outputs).empty())
      {
        outputs["__fake__output__"] = prepared_data.pointer_output;
      }

    auto content = network_butcher::types::Content<type_info_pointer>::make_content(std::move(inputs),
                                                                                    std::move(outputs),
                                                                                    std::move(parameters),
                                                                                    process_node_attributes(node),
                                                                                    std::move(operation_type));

    auto res = node_type(std::move(content));
    res.name = node.name();

    return res;
  }


} // namespace network_butcher::io