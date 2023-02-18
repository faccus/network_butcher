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

#include "onnx.pb.h"

#include "Node_traits.h"
#include "Content.h"
#include "Node.h"

namespace network_butcher_types
{
  /// Just another graph class...
  /// \tparam T Type of the content of the nodes
  template <class T>
  class Graph
  {
  public:
    using Node_Type            = Node<T>;
    using Dependencies_Type    = std::vector<std::pair<node_id_collection_type, node_id_collection_type>>;
    using Node_Collection_Type = std::vector<Node_Type>;
    using Node_Internal_Type   = T;

    Graph() = default;

    Graph(Graph const &) = default;
    Graph &
    operator=(Graph const &) = default;

    Graph(Graph &&) = default;
    Graph &
    operator=(Graph &&) = default;

    /// Construct the graph from the nodes and the map containing the relation
    /// between the id of the input/output with the content
    /// \param v The collection of nodes ordered in an ascending order based on
    /// the id. To work with butcher, the nodes must be sorted in
    /// topological order, according to the Onnx IR specifications.
    /// \param dependencies Node dependencies (input and outputs of every node)
    explicit Graph(Node_Collection_Type v, Dependencies_Type dep = {})
      : nodes(std::move(v))
      , dependencies(std::move(dep))
    {
      for (node_id_type i = 0; i < nodes.size(); ++i)
        nodes[i].id = i;
    }

    /// Get the collection of nodes
    /// \return The vector of nodes
    inline const Node_Collection_Type &
    get_nodes() const
    {
      return nodes;
    }

    /// Get the collection of nodes
    /// \return The vector of nodes
    inline Node_Collection_Type &
    get_nodes_ref() const
    {
      return nodes;
    }

    /// Get the collection of dependencies (as a const reference)
    /// \return The dependencies
    [[nodiscard]] inline const Dependencies_Type &
    get_dependencies() const
    {
      return dependencies;
    }

    /// Get the collection of dependencies (as a reference)
    /// \return The dependencies
    [[nodiscard]] inline Dependencies_Type &
    get_dependencies_ref()
    {
      return dependencies;
    }

    /// Gets the number of nodes
    /// \return The number of nodes
    [[nodiscard]] inline const std::size_t
    size() const
    {
      return nodes.size();
    }

    /// Checks if the graph has nodes
    /// \return True if there are no stored nodes
    [[nodiscard]] inline const std::size_t
    empty() const
    {
      return nodes.empty();
    }

    /// Gets the node with the given id
    /// \param id The id
    /// \return The node
    Node_Type const &
    operator[](int id) const
    {
      return nodes[id];
    }

    /// It remove the nodes with the given id. Note that the id of the nodes in the graph may change
    /// \param nodes_to_remove The ids of the nodes to remove
    void
    remove_nodes(std::set<node_id_type> const &nodes_to_remove)
    {
      std::unordered_map<node_id_type, node_id_type> old_to_new;
      std::set<node_id_type>                         keys;

      Dependencies_Type new_dependencies;
      new_dependencies.reserve(nodes.size() - nodes_to_remove.size());

      Node_Collection_Type new_node_collection;
      new_node_collection.reserve(nodes.size() - nodes_to_remove.size());

      for (std::size_t i = 0, j = 0; i < nodes.size(); ++i)
        {
          if (!nodes_to_remove.contains(i))
            {
              keys.insert(i);
              old_to_new[i] = j;
              new_node_collection.emplace_back(std::move(nodes[i]));
              new_node_collection.back().id = j++;
            }
        }

      for (auto const &key : keys)
        {
          new_dependencies.emplace_back();

          for (auto const &in : dependencies[key].first)
            {
              auto const it = old_to_new.find(in);
              if (it != old_to_new.cend())
                new_dependencies.back().first.emplace(it->second);
            }

          for (auto const &out : dependencies[key].second)
            {
              auto const it = old_to_new.find(out);
              if (it != old_to_new.cend())
                new_dependencies.back().second.emplace(it->second);
            }
        }

      std::swap(new_node_collection, nodes);
      std::swap(dependencies, new_dependencies);
    }

    /// It deletes the nodes and dependencies of the graph
    void
    clear()
    {
      nodes.clear();
      dependencies.clear();
    }


    virtual ~Graph() = default;

  protected:
    /// Vector of all the nodes
    Node_Collection_Type nodes;

    /// Vector that contains all the neighbours of every node (first input, then
    /// output)
    Dependencies_Type dependencies;
  };

  template <class T>
  class Graph<Content<T>>
  {
  public:
    using Dependencies_Type    = std::vector<std::pair<node_id_collection_type, node_id_collection_type>>;
    using Node_Type            = Node<Content<T>>;
    using Node_Collection_Type = std::vector<Node_Type>;
    using Node_Content_Type    = Content<T>;
    using Node_Internal_Type   = T;


    Graph() = default;

    Graph(Graph const &) = default;
    Graph &
    operator=(Graph const &) = default;

    Graph(Graph &&) = default;
    Graph &
    operator=(Graph &&) = default;

    /// Construct the graph from the nodes and the map containing the relation
    /// between the id of the input/output with the content
    /// \param v The collection of nodes ordered in an ascending order based on
    /// the id. To work with butcher, the nodes must be sorted in
    /// topological order, according to the Onnx IR specifications.
    /// \param dependencies Node dependencies (input and outputs of every node)
    explicit Graph(Node_Collection_Type v, Dependencies_Type dep)
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
    explicit Graph(Node_Collection_Type const &v)
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
    explicit Graph(Node_Collection_Type &&v)
      : nodes(std::move(v))
    {
      for (node_id_type i = 0; i < nodes.size(); ++i)
        nodes[i].id = i;

      compute_dependencies();
    }

    /// Get the collection of nodes
    /// \return The vector of nodes
    inline const Node_Collection_Type &
    get_nodes() const
    {
      return nodes;
    }

    /// Get the collection of dependencies (as a const reference)
    /// \return The dependencies
    [[nodiscard]] inline const Dependencies_Type &
    get_dependencies() const
    {
      return dependencies;
    }

    /// Get the collection of dependencies (as a reference)
    /// \return The dependencies
    [[nodiscard]] inline Dependencies_Type &
    get_dependencies_ref()
    {
      return dependencies;
    }

    /// Gets the number of nodes
    /// \return The number of nodes
    [[nodiscard]] inline const std::size_t
    size() const
    {
      return nodes.size();
    }

    /// Checks if the graph has nodes
    /// \return True if there are no stored nodes
    [[nodiscard]] inline const std::size_t
    empty() const
    {
      return nodes.empty();
    }

    /// Gets the node with the given id
    /// \param id The id
    /// \return The node
    Node_Type const &
    operator[](int id) const
    {
      return nodes[id];
    }

    /// It remove the nodes with the given id. Note that the id of the nodes in the graph may change
    /// \param nodes_to_remove The ids of the nodes to remove
    void
    remove_nodes(std::set<node_id_type> const &nodes_to_remove)
    {
      std::unordered_map<node_id_type, node_id_type> old_to_new;
      std::set<node_id_type>                         keys;

      Dependencies_Type new_dependencies;
      new_dependencies.reserve(nodes.size() - nodes_to_remove.size());

      Node_Collection_Type new_node_collection;
      new_node_collection.reserve(nodes.size() - nodes_to_remove.size());

      for (std::size_t i = 0, j = 0; i < nodes.size(); ++i)
        {
          if (!nodes_to_remove.contains(i))
            {
              keys.insert(i);
              old_to_new[i] = j;
              new_node_collection.emplace_back(std::move(nodes[i]));
              new_node_collection.back().id = j++;
            }
        }

      for (auto const &key : keys)
        {
          new_dependencies.emplace_back();

          for (auto const &in : dependencies[key].first)
            {
              auto const it = old_to_new.find(in);
              if (it != old_to_new.cend())
                new_dependencies.back().first.emplace(it->second);
            }

          for (auto const &out : dependencies[key].second)
            {
              auto const it = old_to_new.find(out);
              if (it != old_to_new.cend())
                new_dependencies.back().second.emplace(it->second);
            }
        }

      std::swap(new_node_collection, nodes);
      std::swap(dependencies, new_dependencies);
    }

    void
    clear()
    {
      nodes.clear();
      dependencies.clear();
    }

    virtual ~Graph() = default;

  protected:
    /// Vector of all the nodes
    Node_Collection_Type nodes;

    /// Vector that contains all the neighbours of every node (first input, then
    /// output)
    Dependencies_Type dependencies;


    /// Compute node dependencies
    void
    compute_dependencies();
  };

  template <class T>
  void
  Graph<Content<T>>::compute_dependencies()
  {
    // Reset the dependency vector.
    dependencies = Dependencies_Type();
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
          dependencies[node_id].second.insert(appearance.second.cbegin(), appearance.second.cend());
      }
  }
} // namespace network_butcher_types

#endif // NETWORK_BUTCHER_GRAPH_H
