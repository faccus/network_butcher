#include "onnx_model_reconstructor_helpers.h"

#include <ranges>

namespace network_butcher::io::Onnx_model_reconstructor_helpers
{
  auto
  get_type(const onnx::ModelProto &original_model, const std::string &communication_node_name)
    -> std::optional<Repeatable_field<onnx::ValueInfoProto>::const_iterator>
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


  auto
  prepare_new_graph(const onnx::ModelProto &original_model) -> onnx::GraphProto *
  {
    auto new_graph_pointer = new onnx::GraphProto;

    new_graph_pointer->set_name(original_model.graph().name());
    new_graph_pointer->set_doc_string(original_model.graph().doc_string());

    return new_graph_pointer;
  }


  void
  add_nodes(const std::map<Node_Id_Type, Node_Id_Type>     &link_id_nodeproto,
            const onnx::GraphProto                         &model_graph,
            const std::set<Node_Id_Type>                   &nodes,
            onnx::GraphProto                               *current_edited_graph,
            helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes)
  {
    for (network_butcher::Node_Id_Type node : nodes)
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
  add_node_ios_nodes(onnx::GraphProto                               *graph,
                     onnx::NodeProto                                *node,
                     helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes)
  {
    for (int i = 0; i < node->input_size(); ++i)
      {
        auto const it = preprocessed_ios_nodes.find(node->input(i));

        if (it != preprocessed_ios_nodes.cend())
          {
            auto const &[enum_IO_type, value_info_it, initializer_it] = it->second;
            switch (enum_IO_type)
              {
                  case Input: {
                    auto new_input = graph->add_input();
                    *new_input     = *value_info_it;
                    break;
                  }

                case Fake_ValueInfo:
                  case ValueInfo: {
                    auto new_valueinfo = graph->add_value_info();
                    *new_valueinfo     = *value_info_it;
                    break;
                  }

                  case Initializer: {
                    auto new_initializer = graph->add_initializer();
                    *new_initializer     = *initializer_it;
                    break;
                  }

                  case Initializer_Input: {
                    auto new_input       = graph->add_input();
                    auto new_initializer = graph->add_initializer();

                    *new_initializer = *initializer_it;
                    *new_input       = *value_info_it;
                    break;
                  }

                  case Initializer_ValueInfo: {
                    auto new_valueinfo   = graph->add_value_info();
                    auto new_initializer = graph->add_initializer();

                    *new_initializer = *initializer_it;
                    *new_valueinfo   = *value_info_it;
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
  add_missing_inputs(const onnx::ModelProto                         &original_model,
                     onnx::GraphProto                               *current_edited_graph,
                     helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes)
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
  add_missing_outputs(const onnx::ModelProto                         &original_model,
                      onnx::GraphProto                               *current_edited_graph,
                      helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes)
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


  auto
  process_node_ios_nodes(const onnx::GraphProto &model_graph) -> helper_structures::Preprocessed_Ios_Type
  {
    helper_structures::Preprocessed_Ios_Type res;

    std::set<std::string> previous_outputs;
    for (auto const &node : model_graph.node())
      {
        // Loop through the inputs of the node
        for (auto const &input : node.input())
          {
            auto it      = get_element_from_container(model_graph.input(), input);
            auto it_init = get_element_from_container(model_graph.initializer(), input);

            // Is it in the graph input?
            if (it != model_graph.input().cend())
              {
                res.emplace(input,
                            preprocessed_ios_new_entry(
                              true, false, it_init != model_graph.initializer().cend(), false, it, it_init));
              }
            else
              {
                it = get_element_from_container(model_graph.value_info(), input);

                // Is it valid value info? Or is it a non-initialized (parameter) tensor?
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
                                preprocessed_ios_new_entry(false,
                                                           false,
                                                           it_init != model_graph.initializer().cend(),
                                                           it != model_graph.value_info().cend(),
                                                           it,
                                                           it_init));
                  }
              }
          }

        previous_outputs.insert(node.output().cbegin(), node.output().cend());
      }

    return res;
  }


  auto
  preprocessed_ios_new_entry(bool                                                          found_input,
                             bool                                                          found_value_info,
                             bool                                                          found_initializer,
                             bool                                                          fake_initializer,
                             Repeatable_field<onnx::ValueInfoProto>::const_iterator const &it,
                             Repeatable_field<onnx::TensorProto>::const_iterator const    &init)
    -> helper_structures::Preprocessed_Ios_Type::mapped_type
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

    return helper_structures::Preprocessed_Ios_Type::mapped_type{.tensor_type = elem_type,
                                                                 .value_info  = it,
                                                                 .initializer = init};
  }
} // namespace network_butcher::io::Onnx_model_reconstructor_helpers