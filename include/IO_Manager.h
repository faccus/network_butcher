//
// Created by faccus on 20/02/22.
//

#ifndef NETWORK_BUTCHER_IO_MANAGER_H
#define NETWORK_BUTCHER_IO_MANAGER_H

#include "Butcher.h"
#include "Traits/Graph_traits.h"
#include "Types/Parameters.h"

#include "APSC/GetPot"

#include <sstream>

namespace network_butcher_io
{
  class IO_Manager
  {
  private:
    using Map_IO = std::unordered_map<std::string, type_info_pointer>;

    /// Inserts into the input_map the valid elements contained in collection and
    /// whether they are initialized or not
    /// \param input_map The input_map
    /// \param collection The collection of IO elements
    /// \param initialized The collection of names of the initialized IO elements
    static void
    onnx_io_read(Map_IO                                                         &input_map,
                 google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> const &collection,
                 std::set<std::string> const                                    &initialized);

    /// Inserts into the input_map the valid elements contained in collection and
    /// whether they are initialized or not
    /// \param input_map The input_map
    /// \param collection The collection of IO elements
    /// \param initialized The collection of names of the initialized IO elements
    static void
    onnx_io_read(Map_IO                                                      &input_map,
                 google::protobuf::RepeatedPtrField<onnx::TensorProto> const &collection,
                 std::set<std::string> const                                 &initialized);

    /// It will add to either io_collection or parameters_collection the different
    /// elements of io_name if they are contained into value_infos
    /// \param io_names The collection of names of IO identifiers
    /// \param io_collection The collection of Type_info associated to the IO
    /// elements for the given node
    /// \param parameters_collection The collection of Type_info associated to the
    /// parameters elements for the given node
    /// \param value_infos The collection of IO and parameters elements
    static void
    onnx_process_node(google::protobuf::RepeatedPtrField<std::basic_string<char>> const &io_names,
                      io_collection_type<type_info_pointer>                             &io_collection,
                      io_collection_type<type_info_pointer>                             &parameters_collection,
                      Map_IO const                                                      &value_infos);

    /// It will insert into onnx_io_ids the names of the elements of onnx_io
    /// \param onnx_io A collection of onnx::ValueInfoProto
    /// \param onnx_io_ids The collection of names
    static void
    onnx_populate_id_collection(const google::protobuf::RepeatedPtrField<::onnx::ValueInfoProto> &onnx_io,
                                std::set<std::string>                                            &onnx_io_ids);

    /// It will produce the collection of strings that are contained in
    /// onnx_io_ids and that have an element with the same name in io_collection
    /// \param onnx_io_ids The collection of IO ids
    /// \param io_collection The collection IO/parameters for the given node
    /// \return The collection of "common" names
    static std::vector<std::string>
    get_common_elements(const std::set<std::string> &onnx_io_ids, io_collection_type<type_info_pointer> &io_collection);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights_from_csv_aMLLibrary(graph_type &graph, std::size_t device, std::string const &path);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights_from_csv_operation_time(graph_type &graph, std::size_t device, std::string const &path);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights_from_csv_multi_operation_time(graph_type              &graph,
                                                 std::vector<std::size_t> device,
                                                 std::string const       &path);

  public:
    /// It will return the parameters read from the given file
    /// \param path The configuration file path
    /// \return The collection of parameters
    static Parameters
    read_parameters(std::string const &path);

    /// It will import a neural network as a graph from a given .onnx file
    /// \param path The file path of the .onnx file
    /// \param add_padding_nodes If true, two "fake" nodes will be added at the
    /// beginning of the network and at the end, so that the resulting graph has
    /// a single input and a single output
    /// \param num_devices The number of devices
    /// \return A tuple made by the graph, the onnx::ModelProto for the .onnx file
    /// and a map associating every node in the graph to every node in the model
    static std::tuple<graph_type, onnx::ModelProto, std::map<node_id_type, node_id_type>>
    import_from_onnx(std::string const &path, bool add_padding_nodes = true, std::size_t num_devices = 1);

    /// It will export a given onnx::ModelProto to a file
    /// \param model The onnx::ModelProto
    /// \param path The export file path
    static void
    export_to_onnx(onnx::ModelProto const &model, std::string path);

    /// From a given graph and the associated onnx::ModelProto, it will export the
    /// basic information about every convolutional node in the network
    /// \param graph The graph
    /// \param model The onnx::ModelProto
    /// \param path The export file path
    static void
    export_network_infos_to_csv(graph_type const       &graph,
                                onnx::ModelProto const &model,
                                std::string const      &path = "butcher_predict.csv");


    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param weight_mode The type of .csv file to be imported
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights(Weight_Import_Mode const &weight_mode,
                   graph_type               &graph,
                   std::string const        &path,
                   std::size_t               device);

    /// It will read from a .csv file the collection of weights for the given
    /// graph on the specified device
    /// \param weight_mode The type of .csv file to be imported
    /// \param graph The graph
    /// \param device The device id
    /// \param path The path of the file to be "imported"
    static void
    import_weights(Weight_Import_Mode const &weight_mode,
                   graph_type               &graph,
                   std::string const        &path,
                   std::vector<std::size_t>  devices,
                   std::size_t               index = 0);

    /// Based on the graph and the partitions device/nodes, it will prodice the
    /// "butchered" models.
    /// \tparam Graph The type of the graph
    /// \param partitions The partitions device/nodes
    /// \param original_model The original imported model
    /// \param graph The graph
    /// \param link_id_nodeproto The map that associated every node of the graph to
    /// a node in the imported model
    /// \return The collection of models and the related device
    template <class Graph>
    static std::vector<std::pair<onnx::ModelProto, std::size_t>>
    reconstruct_model(network_butcher_types::Real_Path const     &partitions,
                      onnx::ModelProto const                     &original_model,
                      Graph const                                &graph,
                      std::map<node_id_type, node_id_type> const &link_id_nodeproto);

    /// It will reconstruct the ModelProto objects associated to the different
    /// partitions and it will export them to the directory paths
    /// \param params The collection of parameters
    /// \param graph The graph
    /// \param model The original model
    /// \param link_id_nodeproto The map that associated every node of the graph to
    /// a node in the imported model
    /// \param paths The different partitions to be exported
    static void
    export_network_partitions(const Parameters                                 &params,
                              const graph_type                                 &graph,
                              const onnx::ModelProto                           &model,
                              std::map<node_id_type, node_id_type> const       &link_id_nodeproto,
                              const network_butcher_types::Weighted_Real_Paths &paths);
  };

  template <class Graph>
  std::vector<std::pair<onnx::ModelProto, std::size_t>>
  IO_Manager::reconstruct_model(network_butcher_types::Real_Path const     &partitions,
                                onnx::ModelProto const                     &original_model,
                                Graph const                                &graph,
                                std::map<node_id_type, node_id_type> const &link_id_nodeproto)
  {
    std::vector<std::pair<onnx::ModelProto, std::size_t>> res;

    auto const prepare_new_model = [&original_model](onnx::ModelProto &new_model) {
      new_model.set_doc_string(original_model.doc_string());
      new_model.set_domain(original_model.domain());
      new_model.set_producer_name(original_model.producer_name());
      new_model.set_producer_version(original_model.producer_version());
    };
    auto const prepare_new_graph = [&original_model]() {
      auto new_graph_pointer = new onnx::GraphProto;
      new_graph_pointer->set_name(original_model.graph().name());
      new_graph_pointer->set_doc_string(original_model.graph().doc_string());
      return new_graph_pointer;
    };

    auto const &model_graph = original_model.graph();

    auto const get_type = [&](std::string const &communication_node_name) {
      auto const &input      = original_model.graph().input();
      auto const &output     = original_model.graph().output();
      auto const &value_info = original_model.graph().value_info();

      auto tmp_res = std::find_if(input.cbegin(), input.cend(), [communication_node_name](auto const &ref) {
        return ref.name() == communication_node_name;
      });

      if (tmp_res == original_model.graph().input().end())
        tmp_res = std::find_if(output.cbegin(), output.cend(), [communication_node_name](auto const &ref) {
          return ref.name() == communication_node_name;
        });
      else
        return std::pair{true, tmp_res};

      if (tmp_res == original_model.graph().output().end())
        tmp_res = std::find_if(value_info.cbegin(), value_info.cend(), [communication_node_name](auto const &ref) {
          return ref.name() == communication_node_name;
        });
      else
        return std::pair{true, tmp_res};

      return std::pair{tmp_res != value_info.cend(), tmp_res};
    };

    auto const get_initializer = [&](std::string const &name) {
      auto const &initializer = original_model.graph().initializer();
      auto const  it          = std::find_if(initializer.begin(), initializer.end(), [&name](auto const &proto_tensor) {
        return name == proto_tensor.name();
      });
      return std::pair{it == initializer.cend(), it};
    };

    auto const add_node_ios_to_graph = [&model_graph](onnx::GraphProto *sup_graph, onnx::NodeProto const *node) {
      for (int i = 0; i < node->input_size(); ++i)
        {
          auto it = std::find_if(model_graph.input().begin(),
                                 model_graph.input().end(),
                                 [&node, &i](onnx::ValueInfoProto const &ref) { return node->input(i) == ref.name(); });

          if (it != model_graph.input().end())
            {
              auto const tmp = sup_graph->add_input();
              *tmp           = *it;
            }
          else
            {
              it = std::find_if(model_graph.value_info().begin(),
                                model_graph.value_info().end(),
                                [&node, &i](onnx::ValueInfoProto const &ref) { return node->input(i) == ref.name(); });

              if (it != model_graph.value_info().end())
                {
                  auto const tmp = sup_graph->add_value_info();
                  *tmp           = *it;
                }
            }

          auto init = std::find_if(model_graph.initializer().begin(),
                                   model_graph.initializer().end(),
                                   [&node, &i](onnx::TensorProto const &ref) { return node->input(i) == ref.name(); });
          if (init != model_graph.initializer().end())
            {
              auto const tmp = sup_graph->add_initializer();
              *tmp           = *init;
            }
        }
    };

    auto const add_nodes = [&](std::set<node_id_type> const &nodes, onnx::GraphProto *current_edited_graph) {
      for (auto const &node : nodes)
        {
          auto const it = link_id_nodeproto.find(node);

          if (it == link_id_nodeproto.cend())
            continue;

          auto const tmp = current_edited_graph->add_node();
          *tmp           = model_graph.node(it->second);

          add_node_ios_to_graph(current_edited_graph, tmp);
        }
    };

    auto const add_missing_inputs = [&](onnx::GraphProto *current_edited_graph) {
      for (auto it = current_edited_graph->mutable_node()->begin(); it != current_edited_graph->mutable_node()->end();
           ++it)
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
                  auto [found, tmp_res] = get_type(in);
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
    };

    auto const add_missing_outputs = [&](onnx::GraphProto *current_edited_graph) {
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
                  auto [found, tmp_res] = get_type(out);

                  auto tmp_out = current_edited_graph->add_output();
                  tmp_out->set_name(out);

                  if (found && tmp_res->has_type())
                    {
                      tmp_out->set_allocated_type(new onnx::TypeProto(tmp_res->type()));
                    }
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
} // namespace network_butcher_io


#endif // NETWORK_BUTCHER_IO_MANAGER_H
