//
// Created by faccus on 8/7/22.
//
#include "Onnx_model_reconstructor_helpers.h"

namespace network_butcher::io
{

  std::pair<bool, google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator>
  Onnx_model_reconstructor_helpers::get_type(const onnx::ModelProto &original_model,
                                             const std::string      &communication_node_name)
  {
    auto const &graph = original_model.graph();
    auto        res   = get_element_from_container(graph.input(), communication_node_name);

    if (!res.first)
      {
        res = get_element_from_container(graph.output(), communication_node_name);

        if (!res.first)
          {
            res = get_element_from_container(graph.value_info(), communication_node_name);
          }
      }

    return res;
  }


  void
  Onnx_model_reconstructor_helpers::prepare_new_model(const onnx::ModelProto &original_model,
                                                      onnx::ModelProto       &new_model)
  {
    new_model.set_doc_string(original_model.doc_string());
    new_model.set_domain(original_model.domain());
    new_model.set_producer_name(original_model.producer_name());
    new_model.set_producer_version(original_model.producer_version());
  }


  onnx::GraphProto *
  Onnx_model_reconstructor_helpers::prepare_new_graph(const onnx::ModelProto &original_model)
  {
    auto new_graph_pointer = new onnx::GraphProto;

    new_graph_pointer->set_name(original_model.graph().name());
    new_graph_pointer->set_doc_string(original_model.graph().doc_string());

    return new_graph_pointer;
  }


  std::pair<bool, google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>
  Onnx_model_reconstructor_helpers::get_initializer(const onnx::ModelProto &original_model, const std::string &name)
  {
    return get_element_from_container(original_model.graph().initializer(), name);
  }


  void
  Onnx_model_reconstructor_helpers::add_nodes(
    const std::map<node_id_type, node_id_type>                    &link_id_nodeproto,
    const onnx::GraphProto                                        &model_graph,
    const std::set<node_id_type>                                  &nodes,
    onnx::GraphProto                                              *current_edited_graph,
    Onnx_model_reconstructor_helpers::preprocessed_ios_type const &preprocessed_ios_nodes)
  {
    for (auto const &node : nodes)
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
  Onnx_model_reconstructor_helpers::add_node_ios_nodes(
    onnx::GraphProto                                              *graph,
    const onnx::NodeProto                                         *node,
    Onnx_model_reconstructor_helpers::preprocessed_ios_type const &preprocessed_ios_nodes)
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
  Onnx_model_reconstructor_helpers::add_missing_inputs(const onnx::ModelProto &original_model,
                                                       onnx::GraphProto       *edit_graph)
  {
    auto const &container = edit_graph->input();

    auto const get_node_outputs = [](auto const &node) { return node.output(); };
    auto const get_new_entry    = [edit_graph]() { return edit_graph->add_input(); };

    return helper_add_missing_io(original_model, edit_graph, container, get_node_outputs, get_new_entry, false);
  }


  void
  Onnx_model_reconstructor_helpers::add_missing_outputs(const onnx::ModelProto &original_model,
                                                        onnx::GraphProto       *edit_graph)
  {
    auto const &container = edit_graph->output();

    auto const get_node_outputs = [](auto const &node) { return node.input(); };
    auto const get_new_entry    = [edit_graph]() { return edit_graph->add_output(); };

    return helper_add_missing_io(original_model, edit_graph, container, get_node_outputs, get_new_entry, true);
  }


  Onnx_model_reconstructor_helpers::preprocessed_ios_type
  Onnx_model_reconstructor_helpers::process_node_ios_nodes(const onnx::GraphProto &model_graph)
  {
    Onnx_model_reconstructor_helpers::preprocessed_ios_type res;

    for (auto const &node : model_graph.node())
      {
        // Loop through the inputs of the node
        for (auto const &input : node.input())
          {
            // Has it been already initialized?
            auto [found_initializer, init] = get_element_from_container(model_graph.initializer(), input);

            // Is it in the graph input?
            auto [found_input, it] = get_element_from_container(model_graph.input(), input);
            bool found_value_info  = false;

            if (!found_input)
              {
                // Is it in the graph value infos?
                it = std::find_if(model_graph.value_info().begin(),
                                  model_graph.value_info().end(),
                                  [&input](onnx::ValueInfoProto const &ref) { return input == ref.name(); });

                found_value_info = it != model_graph.value_info().end();
              }

            if (found_initializer)
              {
                auto iterator_pair = std::make_pair(it, init);
                if (found_input)
                  {
                    res.emplace(input, std::make_pair(Initializer_Input, iterator_pair));
                  }
                else if (found_value_info)
                  {
                    res.emplace(input, std::make_pair(Initializer_ValueInfo, iterator_pair));
                  }
                else
                  {
                    res.emplace(input, std::make_pair(Initializer, iterator_pair));
                  }
              }
            else
              {
                auto iterator_pair = std::make_pair(it, model_graph.initializer().cend());
                if (found_input)
                  {
                    res.emplace(input, std::make_pair(Input, iterator_pair));
                  }
                else if (found_value_info)
                  {
                    res.emplace(input, std::make_pair(ValueInfo, iterator_pair));
                  }
                else
                  {
                    res.emplace(input, std::make_pair(NoConnection, iterator_pair));
                  }
              }
          }
      }

    return res;
  }

  void
  Onnx_model_reconstructor_helpers::helper_add_missing_io(
    onnx::ModelProto const                                         &original_model,
    onnx::GraphProto                                               *edit_graph,
    google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> const &container,
    const std::function<google::protobuf::RepeatedPtrField<std::basic_string<char>>(onnx::NodeProto const &)>
                                                  &get_node_io,
    const std::function<onnx::ValueInfoProto *()> &get_new_entry,
    bool                                           reversed)
  {
    using fake_iterator_type =
      Onnx_model_reconstructor_helpers::FakeIterator<google::protobuf::RepeatedPtrField<onnx::NodeProto>>;

    auto const &nodes = edit_graph->mutable_node();

    auto const cond_node = [](auto const &node, auto const &name) { return node == name; };
    auto const cond_init = [](auto const &node, auto const &name) { return node.name() == name; };

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

            // If the input didn't appear, then let's add it!
            if (!ok)
              {
                auto [found, tmp_res] = get_type(original_model, el);
                auto new_entry        = get_new_entry();
                new_entry->set_name(el);

                if (found && tmp_res->has_type())
                  {
                    new_entry->set_allocated_type(new onnx::TypeProto(tmp_res->type()));
                  }
              }
          }
      }
  }
} // namespace network_butcher::io