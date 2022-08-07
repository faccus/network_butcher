//
// Created by faccus on 8/7/22.
//

#ifndef NETWORK_BUTCHER_MODEL_RECONSTRUCTOR_H
#define NETWORK_BUTCHER_MODEL_RECONSTRUCTOR_H

#include "Butcher.h"
#include "Traits/Graph_traits.h"

#include <sstream>

namespace network_butcher_io
{
  class Model_reconstructor
  {
  private:
    static std::pair<bool, google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator>
    get_type(const onnx::ModelProto &original_model, const std::string &communication_node_name);

    static void
    prepare_new_model(const onnx::ModelProto &original_model, onnx::ModelProto &new_model);

    static onnx::GraphProto *
    prepare_new_graph(const onnx::ModelProto &original_model);

    static std::pair<bool, google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>
    get_initializer(const onnx::ModelProto &original_model, const std::string &name);

    static void
    add_node_ios_nodes(const onnx::GraphProto &model_graph, onnx::GraphProto *sup_graph, const onnx::NodeProto *node);

    static void
    add_nodes(const std::map<node_id_type, node_id_type> &link_id_nodeproto,
              const onnx::GraphProto                     &model_graph,
              const std::set<node_id_type>               &nodes,
              onnx::GraphProto                           *current_edited_graph);

    static void
    add_missing_inputs(const onnx::ModelProto &original_model, onnx::GraphProto *current_edited_graph);

    static void
    add_missing_outputs(const onnx::ModelProto &original_model, onnx::GraphProto *current_edited_graph);

  public:
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
  };

  template <class Graph>
  std::vector<std::pair<onnx::ModelProto, std::size_t>>
  Model_reconstructor::reconstruct_model(network_butcher_types::Real_Path const     &partitions,
                                         onnx::ModelProto const                     &original_model,
                                         Graph const                                &graph,
                                         std::map<node_id_type, node_id_type> const &link_id_nodeproto)
  {
    std::vector<std::pair<onnx::ModelProto, std::size_t>> res;

    auto const &model_graph = original_model.graph();

    for (const auto &partition : partitions)
      {
        auto const &device_id = partition.first;
        auto const &node_ids  = partition.second;

        res.emplace_back(onnx::ModelProto(), device_id);

        prepare_new_model(original_model, res.back().first);

        auto current_edited_graph = prepare_new_graph(original_model);

        add_nodes(link_id_nodeproto, model_graph, node_ids, current_edited_graph);

        add_missing_inputs(original_model, current_edited_graph);
        add_missing_outputs(original_model, current_edited_graph);

        res.back().first.set_allocated_graph(current_edited_graph);
      }

    return res;
  }
} // namespace network_butcher_io

#endif // NETWORK_BUTCHER_MODEL_RECONSTRUCTOR_H
