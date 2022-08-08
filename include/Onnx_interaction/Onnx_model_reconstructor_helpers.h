//
// Created by faccus on 8/7/22.
//

#ifndef NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H
#define NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H

#include "../Butcher.h"
#include "../Traits/Graph_traits.h"

#include <sstream>

namespace network_butcher_io
{
  class Onnx_model_reconstructor_helpers
  {
  public:
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
  };
} // namespace network_butcher_io

#endif // NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H
