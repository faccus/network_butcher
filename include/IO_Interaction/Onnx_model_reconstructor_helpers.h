//
// Created by faccus on 8/7/22.
//

#ifndef NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H
#define NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H

#include "Butcher.h"
#include "Graph_traits.h"

#include <sstream>

namespace network_butcher::io
{
  namespace Onnx_model_reconstructor_helpers
  {

    enum IO_Type
    {
      Input,
      ValueInfo,
      Initializer,
      Initializer_Input,
      Initializer_ValueInfo,
      NoConnection
    };

    template <typename T>
    class FakeIterator
    {
    private:
      T::const_iterator         forward;
      T::const_reverse_iterator backward;

      bool reversed;

    public:
      explicit FakeIterator(T::const_iterator it)
        : forward{it}
        , reversed{false}
      {}
      explicit FakeIterator(T::const_reverse_iterator it)
        : backward{it}
        , reversed{true} {};

      FakeIterator &
      operator++()
      {
        if (reversed)
          ++backward;
        else
          ++forward;

        return *this;
      }

      bool
      operator==(FakeIterator const &rhs)
      {
        if (reversed == rhs.reversed)
          {
            if (reversed)
              return backward == rhs.backward;
            else
              return forward == rhs.forward;
          }

        return false;
      }

      bool
      operator!=(FakeIterator const &rhs)
      {
        return !(*this == rhs);
      }

      decltype(*forward) const &
      operator*()
      {
        if (reversed)
          return *backward;
        else
          return *forward;
      }
    };

    using preprocessed_ios_type =
      std::unordered_map<std::string,
                         std::pair<IO_Type,
                                   std::pair<google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator,
                                             google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>>>;


    /// From the container and a name, it will search the element with the specified name in the container
    /// \param container The container
    /// \param communication_node_name The name of the item to be found
    /// \return Found and the position
    template <typename T>
    std::pair<bool, typename google::protobuf::RepeatedPtrField<T>::const_iterator>
    get_element_from_container(google::protobuf::RepeatedPtrField<T> const &container,
                               std::string const                           &communication_node_name)
    {
      auto const tmp_res =
        std::find_if(container.cbegin(), container.cend(), [communication_node_name](auto const &ref) {
          return ref.name() == communication_node_name;
        });

      return std::pair{tmp_res != container.cend(), tmp_res};
    }

    /// From the model and the name of the Tensor, it will return if its found and the position
    /// \param original_model The model
    /// \param communication_node_name THe name of the tensor
    /// \return Found and the position
    std::pair<bool, google::protobuf::RepeatedPtrField<onnx::ValueInfoProto>::const_iterator>
    get_type(const onnx::ModelProto &original_model, const std::string &communication_node_name);


    /// From the original model, it will add basic information to the new_model
    /// \param original_model THe original model
    /// \param new_model The model to be prepared
    void
    prepare_new_model(const onnx::ModelProto &original_model, onnx::ModelProto &new_model);


    /// From the original model, it will return a new graph with the same name and documentation string of the graph
    /// of the original model
    /// \param original_model The original model
    /// \return The "new" graph
    onnx::GraphProto *
    prepare_new_graph(const onnx::ModelProto &original_model);


    std::pair<bool, google::protobuf::RepeatedPtrField<onnx::TensorProto>::const_iterator>
    get_initializer(const onnx::ModelProto &original_model, const std::string &name);


    /// It will add to the graph the inputs of the node to either to input, to value_info or to initializer
    /// \param graph The graph
    /// \param node The node
    /// \param preprocessed_ios_nodes The collection of IOs and initializers of the onnx model graph
    void
    add_node_ios_nodes(onnx::GraphProto            *graph,
                       const onnx::NodeProto       *node,
                       preprocessed_ios_type const &preprocessed_ios_nodes);


    /// It will find the inputs/outputs and/or initializers of a given graph
    /// \param model_graph The graph of the given model
    /// \return A map that associates to the name of the tensor a pair describing if it's a in/out/init and its
    /// position in the object graph
    preprocessed_ios_type
    process_node_ios_nodes(const onnx::GraphProto &model_graph);


    /// It will add a specific collections of nodes of the model_graph to the current_edited_graph
    /// \param link_id_nodeproto The map that associates the id of a node in the internal graph representation to its
    /// id in the original graph
    /// \param model_graph The original graph
    /// \param nodes The (internal) ids of the nodes to be added
    /// \param current_edited_graph The current graph
    /// \param preprocessed_ios_nodes The output of process_node_ios_nodes
    void
    add_nodes(const std::map<node_id_type, node_id_type> &link_id_nodeproto,
              const onnx::GraphProto                     &model_graph,
              const std::set<node_id_type>               &nodes,
              onnx::GraphProto                           *current_edited_graph,
              preprocessed_ios_type const                &preprocessed_ios_nodes);


    void
    helper_add_missing_io(
      onnx::ModelProto const                                         &original_model,
      onnx::GraphProto                                               *edit_graph,
      google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> const &container,
      const std::function<google::protobuf::RepeatedPtrField<std::basic_string<char>>(onnx::NodeProto const &)>
                                                    &get_node_io,
      const std::function<onnx::ValueInfoProto *()> &get_new_entry,
      bool                                           reversed);


    /// Adds the "missing" inputs of the current graph. They represent the new inputs for the new model
    /// \param original_model The original model
    /// \param edit_graph The new graph
    void
    add_missing_inputs(const onnx::ModelProto &original_model, onnx::GraphProto *edit_graph);


    /// Adds the "missing" outputs of the current graph. They represent the new outputs for the new model
    /// \param original_model The original model
    /// \param current_edited_graph The new graph
    void
    add_missing_outputs(const onnx::ModelProto &original_model, onnx::GraphProto *current_edited_graph);

  } // namespace Onnx_model_reconstructor_helpers
} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H
