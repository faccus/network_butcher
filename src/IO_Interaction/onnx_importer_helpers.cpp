//
// Created by faccus on 8/7/22.
//
#include "onnx_importer_helpers.h"

namespace network_butcher::io
{
  auto
  Onnx_importer_helpers::process_node_attributes(onnx::NodeProto const &node)
    -> std::unordered_map<std::string, network_butcher::types::Variant_Attribute>
  {
    using DynamicType = network_butcher::types::Variant_Attribute;
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


  auto
  Onnx_importer_helpers::compute_value_infos(const Repeatable_field<::onnx::ValueInfoProto> &onnx_input,
                                             const Repeatable_field<::onnx::ValueInfoProto> &onnx_output,
                                             const Repeatable_field<::onnx::ValueInfoProto> &onnx_value_info,
                                             const Repeatable_field<::onnx::TensorProto>    &onnx_initializer)
    -> helpers_structures::Processed_Value_Infos_Type
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
    auto it = std::set_difference( // PAR,
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

    return helpers_structures::Processed_Value_Infos_Type{.value_infos      = std::move(value_infos),
                                                          .onnx_inputs_ids  = std::move(onnx_inputs_ids),
                                                          .onnx_outputs_ids = std::move(onnx_outputs_ids)};
  }


  auto
  Onnx_importer_helpers::get_common_elements(const std::set<std::string>           &onnx_io_ids,
                                             Io_Collection_Type<Type_Info_Pointer> &io_collection)
    -> std::vector<Type_Info_Pointer>
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

    std::vector<Type_Info_Pointer> res;
    res.reserve(tmp.size());

    for (auto const &tensor_name : tmp)
      res.push_back(io_collection[tensor_name]);

    return res;
  }


  void
  Onnx_importer_helpers::populate_id_collection(const Repeatable_field<::onnx::ValueInfoProto> &onnx_io,
                                                std::set<std::string>                          &onnx_io_ids)
  {
    std::transform(onnx_io.begin(), onnx_io.end(), std::inserter(onnx_io_ids, onnx_io_ids.end()), [](auto const &el) {
      return el.name();
    });
  }


  void
  Onnx_importer_helpers::read_ios(Onnx_importer_helpers::Map_IO                &input_map,
                                  const Repeatable_field<onnx::ValueInfoProto> &collection,
                                  const std::set<std::string>                  &initialized)
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
  Onnx_importer_helpers::read_ios(Onnx_importer_helpers::Map_IO             &input_map,
                                  const Repeatable_field<onnx::TensorProto> &collection,
                                  const std::set<std::string>               &initialized)
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


  auto
  Onnx_importer_helpers::process_node_ios(const Repeatable_field<std::basic_string<char>> &io_names,
                                          Io_Collection_Type<Type_Info_Pointer>           &parameters_collection,
                                          Map_IO const &value_infos) -> Io_Collection_Type<Type_Info_Pointer>
  {
    Io_Collection_Type<Type_Info_Pointer> res;
    for (auto const &io_name : io_names)
      {
        auto const iterator = value_infos.find(io_name);

        if (iterator != value_infos.cend())
          {
            if (iterator->second->is_initialized())
              parameters_collection.insert({io_name, iterator->second});
            else
              res.insert({io_name, iterator->second});
          }
      }

    return res;
  }


  auto
  Onnx_importer_helpers::prepare_import_from_onnx(const onnx::GraphProto &onnx_graph)
    -> helpers_structures::Prepared_Import_Onnx_Type
  {
    auto [value_infos, onnx_inputs_ids, onnx_outputs_ids] =
      compute_value_infos(onnx_graph.input(), onnx_graph.output(), onnx_graph.value_info(), onnx_graph.initializer());

    return helpers_structures::Prepared_Import_Onnx_Type{
      .value_infos = std::move(value_infos),
      .pointer_input =
        std::make_shared<network_butcher::types::Dense_tensor>(0, std::vector<Onnx_Element_Shape_Type>{}),
      .pointer_output =
        std::make_shared<network_butcher::types::Dense_tensor>(0, std::vector<Onnx_Element_Shape_Type>{}),
      .onnx_inputs_ids  = std::move(onnx_inputs_ids),
      .onnx_outputs_ids = std::move(onnx_outputs_ids)};
  }


  auto
  Onnx_importer_helpers::process_node(
    const onnx::NodeProto                                                      &node,
    const Onnx_importer_helpers::helpers_structures::Prepared_Import_Onnx_Type &prepared_data)
    -> helpers_structures::Process_Node_Output_Type
  {
    auto const &value_infos = prepared_data.value_infos;

    auto operation_type = network_butcher::Utilities::to_lowercase_copy(node.op_type());

    Io_Collection_Type<Type_Info_Pointer> parameters;
    auto                                  inputs  = process_node_ios(node.input(), parameters, value_infos);
    auto                                  outputs = process_node_ios(node.output(), parameters, value_infos);

    auto graph_inputs  = get_common_elements(prepared_data.onnx_inputs_ids, inputs);
    auto graph_outputs = get_common_elements(prepared_data.onnx_outputs_ids, outputs);

    auto res = Converted_Onnx_Graph_Type::Node_Type(
      network_butcher::types::Content<Type_Info_Pointer>(std::move(inputs),
                                                         std::move(outputs),
                                                         std::move(parameters),
                                                         process_node_attributes(node),
                                                         std::move(operation_type)));
    res.name = node.name();

    return helpers_structures::Process_Node_Output_Type{.node   = res,
                                                        .input  = std::move(graph_inputs),
                                                        .output = std::move(graph_outputs)};
  }


} // namespace network_butcher::io