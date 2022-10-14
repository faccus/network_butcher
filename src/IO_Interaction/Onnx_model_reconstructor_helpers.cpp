//
// Created by faccus on 8/7/22.
//
#include "../../include/IO_Interaction/Onnx_model_reconstructor_helpers.h"

std::pair<bool, google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator>
network_butcher_io::Onnx_model_reconstructor_helpers::get_type(const onnx::ModelProto &original_model,
                                                               const std::string      &communication_node_name)
{
  {
    auto const &input   = original_model.graph().input();
    auto const  tmp_res = std::find_if(input.cbegin(), input.cend(), [communication_node_name](auto const &ref) {
      return ref.name() == communication_node_name;
    });

    if (tmp_res != original_model.graph().input().end())
      return std::pair{true, tmp_res};
  }

  {
    auto const &output  = original_model.graph().output();
    auto const  tmp_res = std::find_if(output.cbegin(), output.cend(), [communication_node_name](auto const &ref) {
      return ref.name() == communication_node_name;
    });

    if (tmp_res != original_model.graph().output().end())
      return std::pair{true, tmp_res};
  }

  {
    auto const &value_info = original_model.graph().value_info();
    auto const  tmp_res =
      std::find_if(value_info.cbegin(), value_info.cend(), [communication_node_name](auto const &ref) {
        return ref.name() == communication_node_name;
      });

    return std::pair{tmp_res != value_info.cend(), tmp_res};
  }
}

void
network_butcher_io::Onnx_model_reconstructor_helpers::prepare_new_model(const onnx::ModelProto &original_model,
                                                                        onnx::ModelProto       &new_model)
{
  new_model.set_doc_string(original_model.doc_string());
  new_model.set_domain(original_model.domain());
  new_model.set_producer_name(original_model.producer_name());
  new_model.set_producer_version(original_model.producer_version());
}

onnx::GraphProto *
network_butcher_io::Onnx_model_reconstructor_helpers::prepare_new_graph(const onnx::ModelProto &original_model)
{
  auto new_graph_pointer = new onnx::GraphProto;
  new_graph_pointer->set_name(original_model.graph().name());
  new_graph_pointer->set_doc_string(original_model.graph().doc_string());
  return new_graph_pointer;
}

std::pair<bool, google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>
network_butcher_io::Onnx_model_reconstructor_helpers::get_initializer(const onnx::ModelProto &original_model,
                                                                      const std::string      &name)
{
  auto const &initializer = original_model.graph().initializer();
  auto const  it          = std::find_if(initializer.begin(), initializer.end(), [&name](auto const &proto_tensor) {
    return name == proto_tensor.name();
  });
  return std::pair{it == initializer.cend(), it};
}

void
network_butcher_io::Onnx_model_reconstructor_helpers::add_node_ios_nodes(const onnx::GraphProto &model_graph,
                                                                         onnx::GraphProto       *graph,
                                                                         const onnx::NodeProto  *node)
{
  for (int i = 0; i < node->input_size(); ++i)
    {
      auto it = std::find_if(model_graph.input().begin(),
                             model_graph.input().end(),
                             [&node, &i](onnx::ValueInfoProto const &ref) { return node->input(i) == ref.name(); });

      if (it != model_graph.input().end())
        {
          auto const tmp = graph->add_input();
          *tmp           = *it;
        }
      else
        {
          it = std::find_if(model_graph.value_info().begin(),
                            model_graph.value_info().end(),
                            [&node, &i](onnx::ValueInfoProto const &ref) { return node->input(i) == ref.name(); });

          if (it != model_graph.value_info().end())
            {
              auto const tmp = graph->add_value_info();
              *tmp           = *it;
            }
        }

      auto init = std::find_if(model_graph.initializer().begin(),
                               model_graph.initializer().end(),
                               [&node, &i](onnx::TensorProto const &ref) { return node->input(i) == ref.name(); });
      if (init != model_graph.initializer().end())
        {
          auto const tmp = graph->add_initializer();
          *tmp           = *init;
        }
    }
}

void
network_butcher_io::Onnx_model_reconstructor_helpers::add_nodes(
  const std::map<node_id_type, node_id_type> &link_id_nodeproto,
  const onnx::GraphProto                     &model_graph,
  const std::set<node_id_type>               &nodes,
  onnx::GraphProto                           *current_edited_graph)
{
  for (auto const &node : nodes)
    {
      auto const it = link_id_nodeproto.find(node);

      if (it == link_id_nodeproto.cend())
        continue;

      auto const tmp = current_edited_graph->add_node();
      *tmp           = model_graph.node(it->second);

      add_node_ios_nodes(model_graph, current_edited_graph, tmp);
    }
}

void
network_butcher_io::Onnx_model_reconstructor_helpers::add_missing_inputs(const onnx::ModelProto &original_model,
                                                                         onnx::GraphProto       *current_edited_graph)
{
  for (auto it = current_edited_graph->mutable_node()->begin(); it != current_edited_graph->mutable_node()->end(); ++it)
    {
      auto const &ins = it->input();
      for (auto const &in : ins)
        {
          bool ok = false;

          // Cycle throught the inputs of the graph
          for (int j = 0; j < current_edited_graph->input_size() && !ok; ++j)
            {
              if (current_edited_graph->input(j).name() == in)
                ok = true;
            }

          // Cycle throught the nodes of the graph
          for (auto it2 = current_edited_graph->mutable_node()->begin(); it2 != it; ++it2)
            {
              for (int j = 0; j < it2->output_size() && !ok; ++j)
                {
                  if (it2->output(j) == in)
                    ok = true;
                }
            }

          // If the input didn't appear, then let's add it!
          if (!ok)
            {
              auto [found, tmp_res] = get_type(original_model, in);
              auto tmp_in           = current_edited_graph->add_input();
              tmp_in->set_name(in);

              // Is the input a non-initializer?
              if (found && tmp_res->has_type())
                {
                  tmp_in->set_allocated_type(new onnx::TypeProto(tmp_res->type()));
                }
              /*
              else
                {
                  auto [found, tmp2_res] = get_initializer(in);
                  if(found)
                    {
                      auto tmp_init = current_edited_graph->add_initializer();

                      tmp_init->set_name(in);
                      onnx::TensorProto tt;

                      tt.
                      tt.data_type();


                    }
                } */
            }
        }
    }
}

void
network_butcher_io::Onnx_model_reconstructor_helpers::add_missing_outputs(const onnx::ModelProto &original_model,
                                                                          onnx::GraphProto       *current_edited_graph)
{
  for (auto it = current_edited_graph->mutable_node()->rbegin(); it != current_edited_graph->mutable_node()->rend();
       ++it)
    {
      auto const &outs = it->output();
      for (auto const &out : outs)
        {
          bool ok = false;

          // Cycle throught the outputs of the graph
          for (int j = 0; j < current_edited_graph->output_size() && !ok; ++j)
            {
              if (current_edited_graph->output(j).name() == out)
                ok = true;
            }

          // Cycle throught the nodes of the graph
          for (auto it2 = current_edited_graph->mutable_node()->rbegin(); it2 != it; ++it2)
            {
              for (int j = 0; j < it2->input_size() && !ok; ++j)
                {
                  if (it2->input(j) == out)
                    ok = true;
                }
            }

          // If the output didn't appear, then let's add it!
          if (!ok)
            {
              auto [found, tmp_res] = get_type(original_model, out);

              auto tmp_out = current_edited_graph->add_output();
              tmp_out->set_name(out);

              if (found && tmp_res->has_type())
                {
                  tmp_out->set_allocated_type(new onnx::TypeProto(tmp_res->type()));
                }
            }
        }
    }
}