//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_GRAPH_H
#define NETWORK_BUTCHER_GRAPH_H

#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


#include "../Helpers/Traits/Node_traits.h"
#include "../Helpers/Types/Dense_tensor.h"
#include "../Helpers/Types/Type_info.h"
#include "../Onnx_model/onnx.pb.h"
#include "Node.h"

/// Just another graph class...
/// \tparam T Type of the content of the node
template <class T, typename id_content = io_id_type>
class Graph
{
private:
  /// Compute node dependencies
  void
  compute_dependencies()
  {
    dependencies = std::vector<
      std::pair<node_id_collection_type, node_id_collection_type>>();
    dependencies.resize(nodes.size());

    // Compute appearances of inputs/outputs for a node
    std::unordered_map<io_id_type, node_id_collection_type> input_appearances;
    std::unordered_map<io_id_type, node_id_collection_type> output_appearances;

    for (auto const &node : nodes)
      {
        for (auto &in : node.get_input())
          input_appearances[in].insert(node.get_id());
        for (auto &out : node.get_output())
          output_appearances[out].insert(node.get_id());
      }

    for (auto const &appearance : input_appearances)
      {
        auto const &neib = output_appearances[appearance.first];
        for (auto node_id : appearance.second)
          dependencies[node_id].first.insert(neib.cbegin(), neib.cend());
        for (auto node_id : neib)
          dependencies[node_id].second.insert(appearance.second.cbegin(),
                                              appearance.second.cend());
      }
  }

public:
  /// Vector of all the nodes
  std::vector<node_type> nodes;

  /// Relation between the id of the input/output with the content
  std::map<id_content, T> nodes_content;

  /// Vector that contains all the neighbours of every node (first input, then
  /// output)
  std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
    dependencies;

  Graph() = default;

  /// Construct the graph from the nodes and the map containing the relation
  /// between the id of the input/output with the content
  /// \param v The collection of nodes ordered in an ascending order based on
  /// the id. To work with butcher, the nodes must be sorted in
  /// topological order, according to the Onnx IR specifications.
  /// \param content The map that associated the id of the given node content
  /// (it's different from the id of the node, since multiple nodes can have the
  /// same input) with the content itself (default: {}) \param dep The
  /// dependencies (edges) of each node (default: {}) \param dependencies Enable
  /// automatic computation of the edges (default: true)
  explicit Graph(
    std::vector<node_type>         v,
    const std::map<id_content, T> &content = {},
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
         dep          = {},
    bool dependencies = true)
    : nodes(std::move(v))
    , nodes_content(content)
    , dependencies(std::move(dep))
  {
    if (dependencies)
      compute_dependencies();
  }
};


template <>
class Graph<graph_input_type>
{
private:
  using Map_IO           = std::unordered_map<std::string, io_id_type>;
  using Map_Node_Content = std::map<io_id_type, graph_input_type>;


  /// From a ValueInfoProto (type of an input/output/value_info of the graph)
  /// collection of elements, it will add to the given map the association
  /// between the name of the ValueInfoProto with the respective id, while the
  /// respective type is inserted into nodes_content
  /// \param in Collection of ValueInfoProto
  /// \param parameters The names of the parameters
  /// \param parameters_id The function will add to this set the ids of the
  /// parameters
  void
  onnx_parameters_reader(
    Map_IO                                                         &input_map,
    const google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> &in,
    std::unordered_set<std::string> const                          &parameters,
    std::unordered_set<io_id_type> &parameters_id,
    bool                            ignore_parameters = false)
  {
    for (const auto &param : in)
      {
        if (param.IsInitialized())
          {
            const auto &type = param.type();
            if (type.has_map_type())
              {}

            if (type.has_optional_type())
              {}

            if (type.has_sequence_type())
              {}

            if (type.has_tensor_type())
              {
                auto &obj   = type.tensor_type();
                auto  index = nodes_content.size();

                if (parameters.contains(param.name()))
                  {
                    if (ignore_parameters)
                      continue;
                    parameters_id.insert(index);
                  }

                nodes_content[index]    = std::make_shared<Dense_tensor>(param);
                input_map[param.name()] = index;
              }

            if (type.has_sparse_tensor_type())
              {}
          }
        else
          {}
      }
  }


  /// Compute node dependencies
  void
  compute_dependencies()
  {
    dependencies = std::vector<
      std::pair<node_id_collection_type, node_id_collection_type>>();
    dependencies.resize(nodes.size());

    // Compute appearances of inputs/outputs for a node
    std::unordered_map<io_id_type, node_id_collection_type> input_appearances;
    std::unordered_map<io_id_type, node_id_collection_type> output_appearances;

    for (auto const &node : nodes)
      {
        for (auto &in : node.get_input())
          input_appearances[in].insert(node.get_id());
        for (auto &out : node.get_output())
          output_appearances[out].insert(node.get_id());
      }

    for (auto const &appearance : input_appearances)
      {
        auto const &neib = output_appearances[appearance.first];
        for (auto node_id : appearance.second)
          dependencies[node_id].first.insert(neib.cbegin(), neib.cend());
        for (auto node_id : neib)
          dependencies[node_id].second.insert(appearance.second.cbegin(),
                                              appearance.second.cend());
      }
  }

public:
  /// A vector containing, ordered by id, the different value infos
  Map_Node_Content nodes_content;

  /// Collection of nodes
  std::vector<node_type> nodes;

  /// Vector that contains all the neighbours of every node (first input, then
  /// output)
  std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
    dependencies;


  /// A vector containing, ordered by id, the different operations taking place
  /// in the nodes
  std::vector<operation_id_type> nodes_operations;


  Graph() = default;

  /// Construct the graph from the nodes and the map containing the relation
  /// between the id of the input/output with the content
  /// \param v The collection of nodes ordered in an ascending order based on
  /// the id. To work with butcher, the nodes must be sorted in topological
  /// order, according to the Onnx IR specifications.
  /// \param content The map that associated the id of the given node content
  /// (it's different from the id of the node, since multiple nodes can have the
  /// same input) with the content itself (default: {})
  /// \param dep The dependencies (edges) of each node (default: {})
  /// \param dependencies Enable automatic computation of the edges (default:
  /// true)
  explicit Graph(
    std::vector<node_type> v,
    Map_Node_Content       content = {},
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
         dep          = {},
    bool dependencies = true)
    : nodes(std::move(v))
    , nodes_content(std::move(content))
    , dependencies(std::move(dep))
  {
    if (dependencies)
      compute_dependencies();
  }


  Graph(Graph<graph_input_type> &&) = default;

  /// Construct a graph from a model
  /// \param model Protobuf model
  /// \param ignore_parameters Ignore the inputs/outputs already initialized
  explicit Graph(const onnx::ModelProto &model, bool ignore_parameters = false)
  {
    const auto                    &in_graph = model.graph();
    Map_IO                         io_value_infos_graph;
    std::unordered_set<io_id_type> parameters_id;

    {
      std::unordered_set<std::string> parameters;

      for (auto &p : in_graph.initializer())
        if (p.IsInitialized())
          parameters.insert(p.name());

      onnx_parameters_reader(io_value_infos_graph,
                             in_graph.input(),
                             parameters,
                             parameters_id);

      onnx_parameters_reader(io_value_infos_graph,
                             in_graph.output(),
                             parameters,
                             parameters_id);

      onnx_parameters_reader(io_value_infos_graph,
                             in_graph.value_info(),
                             parameters,
                             parameters_id);
    }

    // Vector of nodes of the graph
    const auto &in_graph_nodes = in_graph.node();

    nodes.reserve(in_graph_nodes.size());
    nodes_operations.reserve(in_graph_nodes.size());

    // Base on the names of the input/output, it will produce the associated
    // type id
    auto process_nodes =
      [&io_value_infos_graph, &parameters_id](
        const google::protobuf::RepeatedPtrField<std::basic_string<char>> &inp,
        std::set<io_id_type>                                              &ing,
        std::set<io_id_type> &params) {
        for (auto const &in : inp)
          {
            auto p     = io_value_infos_graph.find(in);
            bool valid = p != io_value_infos_graph.cend();

            if (valid)
              {
                if (parameters_id.contains(p->second))
                  params.insert(p->second);
                else
                  ing.insert(p->second);
              }
          }
      };

    node_id_type node_index = -1;

    for (const auto &node : in_graph_nodes)
      {
        io_id_collection_type input;
        io_id_collection_type output;
        io_id_collection_type parameters;

        process_nodes(node.input(), input, parameters);
        process_nodes(node.output(), output, parameters);


        if (!(input.empty() && ignore_parameters))
          {
            auto current_index = ++node_index;
            nodes.emplace_back(current_index, input, output, parameters);
            nodes_operations.emplace_back(node.op_type());
          }
      }

    compute_dependencies();
  }


  /// Construct a graph from a Protobuf Model stored at the specified path
  /// \param path The absolute/relative path of the .onnx model
  /// \param ignore_parameters Ignore the inputs/outputs already initialized
  explicit Graph(const std::string &path, bool ignore_parameters = false)
    : Graph(utilities::parse_onnx_file(path), ignore_parameters)
  {}
};


#endif // NETWORK_BUTCHER_GRAPH_H
