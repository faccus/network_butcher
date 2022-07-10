//
// Created by faccus on 20/02/22.
//

#ifndef NETWORK_BUTCHER_IO_MANAGER_H
#define NETWORK_BUTCHER_IO_MANAGER_H

#include "Traits/Graph_traits.h"
#include "../Butcher.h"

#include <sstream>

class IO_Manager
{
private:
  using Map_IO = std::unordered_map<std::string, type_info_pointer>;

  static void
  onnx_io_read(
    Map_IO                                                         &input_map,
    google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> const &collection,
    std::set<std::string> const &initialized);

  static void
  onnx_process_node(
    google::protobuf::RepeatedPtrField<std::basic_string<char>> const &io_names,
    io_collection_type<type_info_pointer> &io_collection,
    io_collection_type<type_info_pointer> &parameters_collection,
    Map_IO const                          &value_infos);

  static void
  onnx_populate_id_collection(
    const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
    std::set<std::string> &onnx_io_ids);

  static std::vector<std::string>
  get_common_elements(const std::set<std::string>           &onnx_io_ids,
                      io_collection_type<type_info_pointer> &io_collection);
public:
  static std::
    tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
    import_from_onnx(std::string const &path,
                     bool               add_padding_nodes = true,
                     std::size_t        num_devices       = 1);

  static void
  export_to_onnx(onnx::ModelProto const &model, std::string path);

  static void
  export_network_infos_to_csv(
    graph_type const       &graph,
    onnx::ModelProto const &model,
    std::string const      &path = "butcher_predict.csv");

  static void
  import_weights_from_csv(graph_type        &graph,
                          std::size_t        device,
                          std::string const &path);

  template <class Graph>
  static std::vector<std::pair<onnx::ModelProto, std::size_t>>
  reconstruct_model(
    Real_Path const                            &partitions,
    onnx::ModelProto const                     &original_model,
    Graph const                                &graph,
    std::map<node_id_type, node_id_type> const &node_collection);
};

template <class Graph>
std::vector<std::pair<onnx::ModelProto, std::size_t>>
IO_Manager::reconstruct_model(
  Real_Path const                      &partitions,
  onnx::ModelProto const               &original_model,
  Graph const                          &graph,
  std::map<node_id_type, node_id_type> const &node_collection)
{
  std::vector<std::pair<onnx::ModelProto, std::size_t>> res;

  std::function<void(onnx::ModelProto &)> prepare_new_model =
    [&original_model](onnx::ModelProto &new_model) {
      new_model.set_doc_string(original_model.doc_string());
      new_model.set_domain(original_model.domain());
      new_model.set_producer_name(original_model.producer_name());
      new_model.set_producer_version(original_model.producer_version());
    };
  std::function<onnx::GraphProto *()> prepare_new_graph = [&original_model]() {
    auto new_graph_pointer = new onnx::GraphProto;
    new_graph_pointer->set_name(original_model.graph().name());
    new_graph_pointer->set_doc_string(original_model.graph().doc_string());
    return new_graph_pointer;
  };

  auto const &model_graph = original_model.graph();

  std::function<google::protobuf::internal::RepeatedPtrIterator<
    const onnx::ValueInfoProto>(std::string const &)>
    get_type = [&](std::string const &communication_node_name) {
      auto tmp_res =
        std::find_if(original_model.graph().input().begin(),
                     original_model.graph().input().end(),
                     [communication_node_name](auto const &ref) {
                       return ref.name() == communication_node_name;
                     });

      if (tmp_res == original_model.graph().input().end())
        tmp_res = std::find_if(original_model.graph().output().begin(),
                               original_model.graph().output().end(),
                               [communication_node_name](auto const &ref) {
                                 return ref.name() == communication_node_name;
                               });
      else
        return tmp_res;

      if (tmp_res == original_model.graph().output().end())
        tmp_res = std::find_if(original_model.graph().value_info().begin(),
                               original_model.graph().value_info().end(),
                               [communication_node_name](auto const &ref) {
                                 return ref.name() == communication_node_name;
                               });

      return tmp_res;
    };

  std::function<void(onnx::GraphProto *, onnx::NodeProto const *)>
    add_node_ios_to_graph = [&model_graph](onnx::GraphProto      *sup_graph,
                                           onnx::NodeProto const *node) {
      for (int i = 0; i < node->input_size(); ++i)
        {
          auto it = std::find_if(model_graph.input().begin(),
                                 model_graph.input().end(),
                                 [&node, &i](onnx::ValueInfoProto const &ref) {
                                   return node->input(i) == ref.name();
                                 });

          if (it != model_graph.input().end())
            {
              auto const tmp = sup_graph->add_input();
              *tmp           = *it;
            }
          else
            {
              it = std::find_if(model_graph.value_info().begin(),
                                model_graph.value_info().end(),
                                [&node, &i](onnx::ValueInfoProto const &ref) {
                                  return node->input(i) == ref.name();
                                });

              if (it != model_graph.value_info().end())
                {
                  auto const tmp = sup_graph->add_value_info();
                  *tmp           = *it;
                }
            }

          auto init = std::find_if(model_graph.initializer().begin(),
                                   model_graph.initializer().end(),
                                   [&node, &i](onnx::TensorProto const &ref) {
                                     return node->input(i) == ref.name();
                                   });
          if (init != model_graph.initializer().end())
            {
              auto const tmp = sup_graph->add_initializer();
              *tmp           = *init;
            }
        }
    };

  std::function<void(std::set<node_id_type> const &, onnx::GraphProto *)>
    add_nodes = [&](std::set<node_id_type> const &nodes,
                    onnx::GraphProto             *current_edited_graph) {
      for (auto const &node : nodes)
        {
          auto const it = node_collection.find(node);

          if(it == node_collection.cend())
            continue;

          auto const tmp = current_edited_graph->add_node();
          *tmp = model_graph.node(it->second);

          add_node_ios_to_graph(current_edited_graph, tmp);
        }
    };

  std::function<void(onnx::GraphProto *)>
    add_missing_inputs = [&](onnx::GraphProto *current_edited_graph) {
      for (auto it = current_edited_graph->mutable_node()->begin();
           it != current_edited_graph->mutable_node()->end();
           ++it)
        {
          auto const &ins = it->input();
          for (auto const &in : ins)
            {
              bool ok = false;

              // Cycle throught the inputs of the graph
              for (int j = 0; j < current_edited_graph->input_size() && !ok;
                   ++j)
                {
                  if (current_edited_graph->input(j).name() == in)
                    ok = true;
                }

              // Cycle throught the nodes of the graph
              for (auto it2 = current_edited_graph->mutable_node()->begin();
                   it2 != it;
                   ++it2)
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
                  auto tmp_in  = current_edited_graph->add_input();
                  auto tmp_res = get_type(in);

                  tmp_in->set_name(in);
                  if (tmp_res != original_model.graph().input().end())
                    tmp_in->set_allocated_type(
                      new onnx::TypeProto(tmp_res->type()));
                }
            }
        }
    };

  std::function<void(onnx::GraphProto *)>
    add_missing_outputs = [&](onnx::GraphProto *current_edited_graph) {
      for (auto it = current_edited_graph->mutable_node()->rbegin();
           it != current_edited_graph->mutable_node()->rend();
           ++it)
        {
          auto const &outs = it->output();
          for (auto const &out : outs)
            {
              bool ok = false;

              // Cycle throught the outputs of the graph
              for (int j = 0; j < current_edited_graph->output_size() && !ok;
                   ++j)
                {
                  if (current_edited_graph->output(j).name() == out)
                    ok = true;
                }

              // Cycle throught the nodes of the graph
              for (auto it2 = current_edited_graph->mutable_node()->rbegin();
                   it2 != it;
                   ++it2)
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
                  auto tmp_out = current_edited_graph->add_output();
                  auto tmp_res = get_type(out);

                  tmp_out->set_name(out);
                  if (tmp_res != original_model.graph().output().end())
                    tmp_out->set_allocated_type(
                      new onnx::TypeProto(tmp_res->type()));
                }
            }
        }
    };

  for (const auto &partition : partitions)
    {
      auto const &device_id = partition.first;
      auto const &node_ids  = partition.second;

      res.emplace_back(onnx::ModelProto(), device_id);
      prepare_new_model(res.back().first);
      auto current_edited_graph = prepare_new_graph();

      add_nodes(node_ids, current_edited_graph);

      add_missing_inputs(current_edited_graph);
      add_missing_outputs(current_edited_graph);

      res.back().first.set_allocated_graph(current_edited_graph);
    }

  return res;
}


#endif // NETWORK_BUTCHER_IO_MANAGER_H
