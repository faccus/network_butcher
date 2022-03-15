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

#include "../Onnx_model/onnx.pb.h"

#include "../Helpers/Traits/Node_traits.h"
#include "Node.h"
#include "../Helpers/Types/Content.h"

/// Just another graph class...
/// \tparam T Type of the content of the node
template <class T>
class Graph
{
protected:
  /// Vector of all the nodes
  std::vector<Node<T>> nodes;

  /// Vector that contains all the neighbours of every node (first input, then
  /// output)
  std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
    dependencies;

public:
  Graph() = default;

  Graph(Graph const &) = default;
  Graph & operator = (Graph const &) = default;

  Graph(Graph &&) = default;
  Graph & operator = (Graph &&) = default;

  /// Construct the graph from the nodes and the map containing the relation
  /// between the id of the input/output with the content
  /// \param v The collection of nodes ordered in an ascending order based on
  /// the id. To work with butcher, the nodes must be sorted in
  /// topological order, according to the Onnx IR specifications.
  /// \param dependencies Node dependencies (input and outputs of every node)
  explicit Graph(
    std::vector<Node<T>> v,
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      dep = {})
    : nodes(std::move(v))
    , dependencies(std::move(dep))
  {
    for (node_id_type i = 0; i < nodes.size(); ++i)
      nodes[i].id = i;
  }

  inline const std::vector<Node<T>> &
  get_nodes() const
  {
    return nodes;
  }

  [[nodiscard]] inline const std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
    &
    get_dependencies() const
  {
    return dependencies;
  }

  virtual ~Graph() = default;
};

template <class T>
class Graph<Content<T>>
{
protected:
  using Node_Type = Node<Content<T>>;

  /// Vector of all the nodes
  std::vector<Node_Type> nodes;

  /// Vector that contains all the neighbours of every node (first input, then
  /// output)
  std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
    dependencies;


  /// Compute node dependencies
  void
  compute_dependencies()
  {
    // Reset the dependency vector.
    dependencies = std::vector<
      std::pair<node_id_collection_type, node_id_collection_type>>();
    dependencies.resize(nodes.size());

    // Compute appearances of inputs/outputs for a node
    std::unordered_map<std::string, node_id_collection_type> input_appearances;
    std::unordered_map<std::string, node_id_collection_type> output_appearances;

    // Check which node has which input/output
    for (auto const &node : nodes)
      {
        auto const &content = node.content;
        for (auto &in : content.get_input())
          input_appearances[in.first].insert(node.get_id());
        for (auto &out : content.get_output())
          output_appearances[out.first].insert(node.get_id());
      }

    // Matched the input of a node to his outputs and viceversa
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

  Graph()              = default;

  Graph(Graph const &) = default;
  Graph & operator = (Graph const &) = default;

  Graph(Graph &&) = default;
  Graph & operator = (Graph &&) = default;

  /// Construct the graph from the nodes and the map containing the relation
  /// between the id of the input/output with the content
  /// \param v The collection of nodes ordered in an ascending order based on
  /// the id. To work with butcher, the nodes must be sorted in
  /// topological order, according to the Onnx IR specifications.
  /// \param dependencies Node dependencies (input and outputs of every node)
  explicit Graph(
    std::vector<Node_Type> v,
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      dep)
    : nodes(std::move(v))
    , dependencies(std::move(dep))
  {
    for (node_id_type i = 0; i < nodes.size(); ++i)
      nodes[i].id = i;
  }

  /// Construct the graph from the nodes and the map containing the relation
  /// between the id of the input/output with the content
  /// \param v The collection of nodes ordered in an ascending order based on
  /// the id. To work with butcher, the nodes must be sorted in
  /// topological order, according to the Onnx IR specifications.
  explicit Graph(std::vector<Node_Type> const &v)
    : nodes(v)
  {
    for (node_id_type i = 0; i < nodes.size(); ++i)
      nodes[i].id = i;

    compute_dependencies();
  }

  /// Construct the graph from the nodes and the map containing the relation
  /// between the id of the input/output with the content
  /// \param v The collection of nodes ordered in an ascending order based on
  /// the id. To work with butcher, the nodes must be sorted in
  /// topological order, according to the Onnx IR specifications.
  explicit Graph(std::vector<Node_Type> &&v)
    : nodes(std::move(v))
  {
    for (node_id_type i = 0; i < nodes.size(); ++i)
      nodes[i].id = i;

    compute_dependencies();
  }

  inline const std::vector<Node_Type> &
  get_nodes() const
  {
    return nodes;
  }

  inline const std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
    &
    get_dependencies() const
  {
    return dependencies;
  }

  virtual ~Graph() = default;
};

#endif // NETWORK_BUTCHER_GRAPH_H
