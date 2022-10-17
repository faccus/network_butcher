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
    enum IO_Type
    {
      Input,
      ValueInfo,
      Initializer,
      Initializer_Input,
      Initializer_ValueInfo,
      NoConnection
    };

    /// From the model and the name of the Tensor, it will return if its found and the position
    /// \param original_model The model
    /// \param communication_node_name THe name of the tensor
    /// \return Found and the position
    static std::pair<bool, google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator>
    get_type(const onnx::ModelProto &original_model, const std::string &communication_node_name);

    /// From the original model, it will add basic information to the new_model
    /// \param original_model THe original model
    /// \param new_model The model to be prepared
    static void
    prepare_new_model(const onnx::ModelProto &original_model, onnx::ModelProto &new_model);

    /// From the original model, it will return a new graph with the same name and documentation string of the graph of
    /// the original model
    /// \param original_model The original model
    /// \return The "new" graph
    static onnx::GraphProto *
    prepare_new_graph(const onnx::ModelProto &original_model);

    static std::pair<bool, google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>
    get_initializer(const onnx::ModelProto &original_model, const std::string &name);

    /// It will add to the graph the inputs of the node to either to input, to value_info or to initializer
    /// \param model_graph The model
    /// \param graph The graph
    /// \param node The node
    static void
    add_node_ios_nodes(
      onnx::GraphProto      *graph,
      const onnx::NodeProto *node,
      std::unordered_map<
        std::string,
        std::pair<network_butcher_io::Onnx_model_reconstructor_helpers::IO_Type,
                  std::pair<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator,
                            google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>>> const
        &preprocessed_ios_nodes);

    static std::unordered_map<
      std::string,
      std::pair<IO_Type,
                std::pair<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator,
                          google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>>>
    process_node_ios_nodes(const onnx::GraphProto &model_graph);

    static void
    add_nodes(const std::map<node_id_type, node_id_type> &link_id_nodeproto,
              const onnx::GraphProto                     &model_graph,
              const std::set<node_id_type>               &nodes,
              onnx::GraphProto                           *current_edited_graph,
              std::unordered_map<
                std::string,
                std::pair<network_butcher_io::Onnx_model_reconstructor_helpers::IO_Type,
                          std::pair<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator,
                                    google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>>> const
                &preprocessed_ios_nodes);

    static void
    add_missing_inputs(const onnx::ModelProto &original_model, onnx::GraphProto *current_edited_graph);

    static void
    add_missing_outputs(const onnx::ModelProto &original_model, onnx::GraphProto *current_edited_graph);
  };
} // namespace network_butcher_io

#endif // NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H
