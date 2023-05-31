//
// Created by faccus on 8/7/22.
//
#include "Onnx_model_reconstructor_helpers.h"

#include <ranges>

namespace network_butcher::io::Onnx_model_reconstructor_helpers
{
  std::optional<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator>
  get_type(const onnx::ModelProto &original_model, const std::string &communication_node_name)
  {
    auto const &graph = original_model.graph();

    auto res = get_element_from_container(graph.input(), communication_node_name);

    if (res != graph.input().cend())
      return res;

    res = get_element_from_container(graph.output(), communication_node_name);

    if (res != graph.output().cend())
      return res;


    res = get_element_from_container(graph.value_info(), communication_node_name);

    if (res != graph.value_info().cend())
      return res;

    return {};
  }


  void
  prepare_new_model(const onnx::ModelProto &original_model, onnx::ModelProto &new_model)
  {
    new_model.set_doc_string(original_model.doc_string());
    new_model.set_domain(original_model.domain());
    new_model.set_producer_name(original_model.producer_name());
    new_model.set_producer_version(original_model.producer_version());
  }


  onnx::GraphProto *
  prepare_new_graph(const onnx::ModelProto &original_model)
  {
    auto new_graph_pointer = new onnx::GraphProto;

    new_graph_pointer->set_name(original_model.graph().name());
    new_graph_pointer->set_doc_string(original_model.graph().doc_string());

    return new_graph_pointer;
  }


  void
  add_nodes(const std::map<node_id_type, node_id_type>                    &link_id_nodeproto,
            const onnx::GraphProto                                        &model_graph,
            const std::set<node_id_type>                                  &nodes,
            onnx::GraphProto                                              *current_edited_graph,
            Onnx_model_reconstructor_helpers::preprocessed_ios_type const &preprocessed_ios_nodes)
  {
    for (network_butcher::node_id_type node : nodes)
      {
        auto const it = link_id_nodeproto.find(node);

        if (it == link_id_nodeproto.cend())
          continue;

        auto tmp = current_edited_graph->add_node();
        *tmp     = model_graph.node(it->second);

        add_node_ios_nodes(current_edited_graph, tmp, preprocessed_ios_nodes);
      }
  }


  void
  add_node_ios_nodes(onnx::GraphProto            *graph,
                     onnx::NodeProto             *node,
                     preprocessed_ios_type const &preprocessed_ios_nodes)
  {
    for (int i = 0; i < node->input_size(); ++i)
      {
        auto const it = preprocessed_ios_nodes.find(node->input(i));

        if (it != preprocessed_ios_nodes.cend())
          {
            auto const &[enum_IO_type, tensors] = it->second;
            switch (enum_IO_type)
              {
                  case Input: {
                    auto new_input = graph->add_input();
                    *new_input     = *tensors.first;
                    break;
                  }

                case Fake_ValueInfo:
                  case ValueInfo: {
                    auto new_valueinfo = graph->add_value_info();
                    *new_valueinfo     = *tensors.first;
                    break;
                  }

                  case Initializer: {
                    auto new_initializer = graph->add_initializer();
                    *new_initializer     = *tensors.second;
                    break;
                  }

                  case Initializer_Input: {
                    auto new_input       = graph->add_input();
                    auto new_initializer = graph->add_initializer();

                    *new_initializer = *tensors.second;
                    *new_input       = *tensors.first;
                    break;
                  }

                  case Initializer_ValueInfo: {
                    auto new_valueinfo   = graph->add_value_info();
                    auto new_initializer = graph->add_initializer();

                    *new_initializer = *tensors.second;
                    *new_valueinfo   = *tensors.first;
                    break;
                  }

                  default: {
                    break;
                  }
              }
          }
      }
  }


  void
  add_missing_inputs(const onnx::ModelProto      &original_model,
                     onnx::GraphProto            *current_edited_graph,
                     preprocessed_ios_type const &preprocessed_ios_nodes)
  {
    auto const &container = current_edited_graph->input();

    auto const get_node_inputs  = [](auto const &node) { return node.input(); };
    auto const get_node_outputs = [](auto const &node) { return node.output(); };

    auto const get_new_entry = [current_edited_graph]() { return current_edited_graph->add_input(); };

    return helper_add_missing_io<false>(original_model,
                                        current_edited_graph,
                                        container,
                                        get_node_outputs,
                                        get_node_inputs,
                                        get_new_entry,
                                        preprocessed_ios_nodes);
  }


  void
  add_missing_outputs(const onnx::ModelProto      &original_model,
                      onnx::GraphProto            *current_edited_graph,
                      preprocessed_ios_type const &preprocessed_ios_nodes)
  {
    auto const &container = current_edited_graph->output();

    auto const get_node_inputs  = [](auto const &node) { return node.input(); };
    auto const get_node_outputs = [](auto const &node) { return node.output(); };

    auto const get_new_entry = [current_edited_graph]() { return current_edited_graph->add_output(); };

    return helper_add_missing_io<true>(original_model,
                                       current_edited_graph,
                                       container,
                                       get_node_inputs,
                                       get_node_outputs,
                                       get_new_entry,
                                       preprocessed_ios_nodes);
  }


  preprocessed_ios_type
  process_node_ios_nodes(const onnx::GraphProto &model_graph)
  {
    preprocessed_ios_type res;

    std::set<std::string> previous_outputs;
    for (auto const &node : model_graph.node())
      {
        // Loop through the inputs of the node
        for (auto const &input : node.input())
          {
            // Is it in the graph input?
            auto it      = get_element_from_container(model_graph.input(), input);
            auto it_init = get_element_from_container(model_graph.initializer(), input);

            if (it != model_graph.input().cend())
              {
                res.emplace(input,
                            preprocessed_ios_new_entry(
                              true, false, it_init != model_graph.initializer().cend(), false, it, it_init));
              }
            else
              {
                it = get_element_from_container(model_graph.value_info(), input);
                if (previous_outputs.contains(input))
                  {
                    res.emplace(input,
                                preprocessed_ios_new_entry(false,
                                                           it != model_graph.value_info().cend(),
                                                           it_init != model_graph.initializer().cend(),
                                                           false,
                                                           it,
                                                           it_init));
                  }
                else
                  {
                    res.emplace(input,
                                preprocessed_ios_new_entry(
                                  false, false, it_init != model_graph.initializer().cend(), true, it, it_init));
                  }
              }
          }

        previous_outputs.insert(node.output().cbegin(), node.output().cend());
      }

    return res;
  }


  preprocessed_ios_type::mapped_type
  preprocessed_ios_new_entry(bool found_input,
                             bool found_value_info,
                             bool found_initializer,
                             bool fake_initializer,
                             google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator const &it,
                             google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator const    &init)
  {
    IO_Type elem_type;

    if (fake_initializer)
      {
        elem_type = found_initializer ? Initializer : Fake_ValueInfo;
      }
    else if (found_input)
      {
        elem_type = found_initializer ? Initializer_Input : Input;
      }
    else if (found_value_info)
      {
        elem_type = found_initializer ? Initializer_ValueInfo : ValueInfo;
      }
    else
      {
        elem_type = found_initializer ? Initializer : NoConnection;
      }

    return std::make_pair(elem_type, std::make_pair(it, init));
  }
} // namespace network_butcher::io::Onnx_model_reconstructor_helpers