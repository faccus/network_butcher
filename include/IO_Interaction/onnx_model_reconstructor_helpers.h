#ifndef NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H
#define NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H

#include "butcher.h"
#include "graph_traits.h"

#include <sstream>

namespace network_butcher::io::Onnx_model_reconstructor_helpers
{
  /// Enumerator for the type of the tensor in the Onnx graph
  enum IO_Type
  {
    // Graph input
    Input,

    // Either an input of a node or an output
    ValueInfo,

    // Already initialized value
    Initializer,

    // Already initialized input of the graph
    Initializer_Input,

    // Already initialized io of a node
    Initializer_ValueInfo,

    // Formally an input of a node, it's not an output of any node. Thus, the model analyzed is not
    // trained and this ValueInfo is not initialized. It should be treated by the Node point of view as
    // a ValueInfo, while by the model point of view as an initializer (thus, it cannot be a tensor to
    // be sent to another device)
    Fake_ValueInfo,

    // Unknown connection (neither an input nor an output nor a ValueInfo....). It should be impossible
    NoConnection
  };


  /// Collection of helper structs (used instead of tuples to improve readability)
  namespace helper_structures
  {
    struct IO_Tensors_Iterators_Type
    {
      IO_Type                                                tensor_type;
      Repeatable_field<onnx::ValueInfoProto>::const_iterator value_info;
      Repeatable_field<onnx::TensorProto>::const_iterator    initializer;
    };

    /// Map that associates to the name of the tensor a pair describing if it's a input/output/initializer and its
    /// iterator
    using Preprocessed_Ios_Type = std::unordered_map<std::string, IO_Tensors_Iterators_Type>;
  } // namespace helper_structures


  /// It will search the element with the specified name in the container
  /// \param container The container
  /// \param communication_node_name The name of the item to be found
  /// \return The iterator to the tensor (or to the end  of the container if it was not found)
  template <typename T>
  auto
  get_element_from_container(Repeatable_field<T> const &container, std::string const &communication_node_name)
    -> Repeatable_field<T>::const_iterator
  {
    return std::find_if(container.cbegin(), container.cend(), [communication_node_name](auto const &ref) {
      return ref.name() == communication_node_name;
    });
  }

  /// From the model and the name of the ValueInfoProto, it will return if its found and the position
  /// \param original_model The model
  /// \param communication_node_name The name of the tensor
  /// \return Optional of constant iterator to the tensor
  auto
  get_type(const onnx::ModelProto &original_model, const std::string &communication_node_name)
    -> std::optional<Repeatable_field<onnx::ValueInfoProto>::const_iterator>;


  /// From the original model, it will add basic information to the new_model
  /// \param original_model The original model
  /// \param new_model The model to be prepared
  void
  prepare_new_model(const onnx::ModelProto &original_model, onnx::ModelProto &new_model);


  /// From the original model, it will return a new graph with the same name and documentation string of the graph
  /// of the original model
  /// \param original_model The original model
  /// \return The "new" graph
  auto
  prepare_new_graph(const onnx::ModelProto &original_model) -> onnx::GraphProto *;


  /// It will add a specific collections of nodes of the model_graph to the current_edited_graph
  /// \param link_id_nodeproto The map that associates the id of a node in the internal graph representation to its
  /// id in the original graph
  /// \param model_graph The original graph
  /// \param nodes The (internal) ids of the nodes to be added
  /// \param current_edited_graph The current graph
  /// \param preprocessed_ios_nodes The output of process_node_ios_nodes
  void
  add_nodes(const std::map<Node_Id_Type, Node_Id_Type>     &link_id_nodeproto,
            const onnx::GraphProto                         &model_graph,
            const std::set<Node_Id_Type>                   &nodes,
            onnx::GraphProto                               *current_edited_graph,
            helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes);


  /// It will add to the graph the inputs of the node to either to input, to value_info or to initializer
  /// \param graph The graph
  /// \param node The node
  /// \param preprocessed_ios_nodes The collection of IOs and initializers of the onnx model graph
  void
  add_node_ios_nodes(onnx::GraphProto                               *graph,
                     onnx::NodeProto                                *node,
                     helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes);


  /// Adds the "missing" inputs of the current graph. They represent the new inputs for the new model
  /// \param original_model The original model
  /// \param current_edited_graph The new graph
  /// \param preprocessed_ios_nodes The output of process_node_ios_nodes
  void
  add_missing_inputs(const onnx::ModelProto                         &original_model,
                     onnx::GraphProto                               *current_edited_graph,
                     helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes);


  /// Adds the "missing" outputs of the current graph. They represent the new outputs for the new model
  /// \param original_model The original model
  /// \param current_edited_graph The new graph
  /// \param preprocessed_ios_nodes The output of process_node_ios_nodes
  void
  add_missing_outputs(const onnx::ModelProto                         &original_model,
                      onnx::GraphProto                               *current_edited_graph,
                      helper_structures::Preprocessed_Ios_Type const &preprocessed_ios_nodes);


  /// It will find the inputs/outputs and/or initializers of a given graph
  /// \param model_graph The graph of the given model
  /// \return A map that associates to the name of the tensor a pair describing if it's a in/out/init and its
  /// position in the object graph
  auto
  process_node_ios_nodes(const onnx::GraphProto &model_graph) -> helper_structures::Preprocessed_Ios_Type;


  /// It will package a tensor and its initializer in a IO_Tensors_Iterators_Type
  /// \param model_graph The graph of the given model
  /// \param found_input True if the iterator "it" comes from the graph input
  /// \param found_value_info True if the iterator is from a node input (that is not an input of the model)
  /// \param found_initializer True if the input value is initialized
  /// \param fake_initializer True if the iterator "it" represents an input of a node that is not an output of any node
  /// (nor a graph input). It may be an initializer
  /// \param it The iterator to the ValueInfoProto
  /// \param init The initializer iterator
  /// \return A IO_Tensors_Iterators_Type, containing the "type" of tensor and the iterator to both the value info and
  /// the initializer
  auto
  preprocessed_ios_new_entry(bool                                                          found_input,
                             bool                                                          found_value_info,
                             bool                                                          found_initializer,
                             bool                                                          fake_initializer,
                             Repeatable_field<onnx::ValueInfoProto>::const_iterator const &it,
                             Repeatable_field<onnx::TensorProto>::const_iterator const    &init)
    -> helper_structures::Preprocessed_Ios_Type::mapped_type;


  /// Helper function used to add the missing inputs/outputs of the graph
  /// \tparam reversed Should the nodes be cycled in the forward or reverse order?
  /// \param  original_model The original model
  /// \param  edit_graph The onnx graph of the new model
  /// \param  container Either the input or the output of the graph
  /// \param  get_io A function that given a node, it returns the inputs/outputs of the node
  /// \param  get_second_io A function that given a node, it returns the outputs/inputs of the node
  /// \param  get_new_entry A function that adds either a new input or a new output to the graph
  /// \param preprocessed_ios_nodes The output of process_node_ios_nodes
  template <bool reversed>
  void
  helper_add_missing_io(
    onnx::ModelProto const                                                                  &original_model,
    onnx::GraphProto                                                                        *edit_graph,
    Repeatable_field<onnx::ValueInfoProto> const                                            &container,
    std::function<Repeatable_field<std::basic_string<char>>(onnx::NodeProto const &)> const &get_io,
    std::function<Repeatable_field<std::basic_string<char>>(onnx::NodeProto const &)> const &get_node_container,
    std::function<onnx::ValueInfoProto *()> const                                           &get_new_entry,
    helper_structures::Preprocessed_Ios_Type const                                          &preprocessed_ios_nodes)
  {
    // Get the nodes
    auto const &nodes = edit_graph->mutable_node();

    // Simple node comparisons: name v name
    auto const comp_names = [](auto const &node, auto const &name) { return node == name; };

    // Simple node comparisons: node v name
    auto const comp_node_name = [](auto const &node, auto const &name) { return node.name() == name; };

    // Checks if the specified element of the container has the same name as the input string
    auto const checkout = [](std::string const &name, auto const &container, auto const &cond_func) {
      for (auto it = container.cbegin(); it != container.cend(); ++it)
        {
          if (cond_func(*it, name))
            return true;
        }

      return false;
    };

    auto const begin = [&nodes]() {
      if constexpr (reversed)
        {
          return nodes->rbegin();
        }
      else
        {
          return nodes->begin();
        }
    };
    auto const end = [&nodes]() {
      if constexpr (reversed)
        {
          return nodes->rend();
        }
      else
        {
          return nodes->end();
        }
    };

    auto const start = begin();

    // Cycle through the nodes (either forward or backward)
    for (auto it = start; it != end(); ++it)
      {
        // inner_container is the collection containing ios that, if they are not already in the graph, must be inserted
        auto const &inner_container = get_node_container(*it);
        for (auto const &el : inner_container)
          {
            // Check if it's an input/output of the original model
            bool ok = checkout(el, container, comp_node_name);

            // Check the already processed nodes for the missing io
            for (auto it2 = start; !ok && it != it2; ++it2)
              ok = checkout(el, get_io(*it2), comp_names);

            // If the input/output didn't appear, then let's add it!
            if (!ok)
              {
                auto tmp_it = preprocessed_ios_nodes.find(el);

                if (tmp_it != preprocessed_ios_nodes.cend() && tmp_it->second.tensor_type == IO_Type::Fake_ValueInfo)
                  continue;

                auto tmp_res = get_type(original_model, el);

                if (tmp_res.has_value() && tmp_res.value()->has_type())
                  {
                    auto new_entry = get_new_entry();
                    new_entry->set_name(el);

                    new_entry->set_allocated_type(new onnx::TypeProto(tmp_res.value()->type()));
                  }
              }
          }
      }
  }
} // namespace network_butcher::io::Onnx_model_reconstructor_helpers

#endif // NETWORK_BUTCHER_ONNX_MODEL_RECONSTRUCTOR_HELPERS_H
