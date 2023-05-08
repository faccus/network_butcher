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

    if (res)
      return res;

    res = get_element_from_container(graph.output(), communication_node_name);

    if (res)
      return res;

    return get_element_from_container(graph.value_info(), communication_node_name);
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
    for (unsigned long node : nodes)
      {
        auto const it = link_id_nodeproto.find(node);

        if (it == link_id_nodeproto.cend())
          continue;

        auto const tmp = current_edited_graph->add_node();
        *tmp           = model_graph.node(it->second);

        add_node_ios_nodes(current_edited_graph, tmp, preprocessed_ios_nodes);
      }
  }


  void
  add_node_ios_nodes(onnx::GraphProto            *graph,
                     const onnx::NodeProto       *node,
                     preprocessed_ios_type const &preprocessed_ios_nodes)
  {
    for (int i = 0; i < node->input_size(); ++i)
      {
        auto const it = preprocessed_ios_nodes.find(node->input(i));

        if (it != preprocessed_ios_nodes.cend())
          {
            switch (it->second.first)
              {
                  case Input: {
                    auto new_input = graph->add_input();
                    *new_input     = *it->second.second.first;
                    break;
                  }

                  case ValueInfo: {
                    auto new_valueinfo = graph->add_value_info();
                    *new_valueinfo     = *it->second.second.first;
                    break;
                  }
                  case Initializer: {
                    auto new_initializer = graph->add_initializer();
                    *new_initializer     = *it->second.second.second;
                    break;
                  }

                  case Initializer_Input: {
                    auto new_input       = graph->add_input();
                    auto new_initializer = graph->add_initializer();

                    *new_initializer = *it->second.second.second;
                    *new_input       = *it->second.second.first;
                    break;
                  }

                  case Initializer_ValueInfo: {
                    auto new_valueinfo   = graph->add_value_info();
                    auto new_initializer = graph->add_initializer();

                    *new_initializer = *it->second.second.second;
                    *new_valueinfo   = *it->second.second.first;
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
  add_missing_inputs(const onnx::ModelProto &original_model, onnx::GraphProto *current_edited_graph)
  {
    auto const &container = current_edited_graph->input();

    auto const get_node_outputs = [](auto const &node) { return node.output(); };
    auto const get_new_entry    = [current_edited_graph]() { return current_edited_graph->add_input(); };

    return helper_add_missing_io(
      original_model, current_edited_graph, container, get_node_outputs, get_new_entry, false);
  }


  void
  add_missing_outputs(const onnx::ModelProto &original_model, onnx::GraphProto *current_edited_graph)
  {
    auto const &container = current_edited_graph->output();

    auto const get_node_outputs = [](auto const &node) { return node.input(); };
    auto const get_new_entry    = [current_edited_graph]() { return current_edited_graph->add_output(); };

    return helper_add_missing_io(
      original_model, current_edited_graph, container, get_node_outputs, get_new_entry, true);
  }


  preprocessed_ios_type
  process_node_ios_nodes(const onnx::GraphProto &model_graph)
  {
    preprocessed_ios_type res;

    for (auto const &node : model_graph.node())
      {
        // Loop through the inputs of the node
        for (auto const &input : node.input())
          {
            // Is it in the graph input?
            auto it = get_element_from_container(model_graph.input(), input);

            if (it)
              {
                res.emplace(input,
                            preprocessed_ios_new_entry(
                              model_graph, true, get_element_from_container(model_graph.initializer(), input), it));
              }
            else
              {
                res.emplace(input,
                            preprocessed_ios_new_entry(model_graph,
                                                       false,
                                                       get_element_from_container(model_graph.initializer(), input),
                                                       get_element_from_container(model_graph.value_info(), input)));
              }
          }
      }

    return res;
  }


  preprocessed_ios_type::mapped_type
  preprocessed_ios_new_entry(
    const onnx::GraphProto                                                                        &model_graph,
    bool                                                                                           found_input,
    std::optional<google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator> const    &init,
    std::optional<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator> const &it)
  {
    auto init_it = init ? init.value() : model_graph.initializer().cend();

    if (found_input)
      {
        return std::make_pair(init ? Initializer_Input : Input, std::make_pair(it.value(), init_it));
      }
    else if (it)
      {
        return std::make_pair(init ? Initializer_ValueInfo : ValueInfo, std::make_pair(it.value(), init_it));
      }
    else
      {
        return std::make_pair(init ? Initializer : NoConnection,
                              std::make_pair(model_graph.value_info().cend(), init_it));
      }
  }


  void
  helper_add_missing_io(
    onnx::ModelProto const                                         &original_model,
    onnx::GraphProto                                               *edit_graph,
    google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> const &container,
    const std::function<google::protobuf::RepeatedPtrField<std::basic_string<char>>(onnx::NodeProto const &)>
                                                  &get_node_io,
    const std::function<onnx::ValueInfoProto *()> &get_new_entry,
    bool                                           reversed)
  {
    using fake_iterator_type = BidirectionalCustomIterator<google::protobuf::RepeatedPtrField<onnx::NodeProto>>;

    auto const &nodes = edit_graph->mutable_node();

    auto const cond_node = [](auto const &node, auto const &name) { return node == name; };
    auto const cond_init = [](auto const &node, auto const &name) { return node.name() == name; };

    // Checks if the specified element of the container has the same name as the input string
    auto const checkout = [](std::string const &name, auto const &container, auto const &cond_func) {
      for (auto it = container.cbegin(); it != container.cend(); ++it)
        {
          if (cond_func(*it, name))
            return true;
        }

      return false;
    };

    auto const start = reversed ? fake_iterator_type(nodes->rbegin()) : fake_iterator_type(nodes->begin());
    auto const end   = reversed ? fake_iterator_type(nodes->rend()) : fake_iterator_type(nodes->end());

    for (auto it = start; it != end; ++it)
      {
        auto const &inner_container = (*it).input();
        for (auto const &el : inner_container)
          {
            bool ok = checkout(el, container, cond_init);

            for (auto it2 = start; it != it2 && !ok; ++it2)
              ok = checkout(el, get_node_io(*it2), cond_node);

            // If the input/output didn't appear, then let's add it!
            if (!ok)
              {
                auto tmp_res   = get_type(original_model, el);
                auto new_entry = get_new_entry();
                new_entry->set_name(el);

                if (tmp_res.has_value() && tmp_res.value()->has_type())
                  {
                    new_entry->set_allocated_type(new onnx::TypeProto(tmp_res.value()->type()));
                  }
              }
          }
      }
  }
} // namespace network_butcher::io