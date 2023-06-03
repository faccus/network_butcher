//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_GRAPH_H
#define NETWORK_BUTCHER_GRAPH_H

#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>

#include "onnx.pb.h"

#include "content.h"
#include "node.h"

#include "node_traits.h"

namespace network_butcher::types
{
  /// Just another graph class...
  /// \tparam T Type of the content of the nodes
  template <typename Template_Node_Type = Node>
    requires std::is_base_of_v<Node, Template_Node_Type> || std::is_same_v<Node, Template_Node_Type>
  class Graph
  {
  public:
    using Neighbours_Type      = std::vector<std::pair<Node_Id_Collection_Type, Node_Id_Collection_Type>>;
    using Node_Type            = Template_Node_Type;
    using Node_Collection_Type = std::vector<Node_Type>;

    Graph() = default;

    /// Construct a new Graph object
    /// \param v The collection of nodes ordered in an ascending order based on the id. To work with butcher, the
    /// nodes must be sorted in topological order, according to the Onnx IR specifications.
    /// \param dep Node neighbours (input and outputs of every node)
    template <typename A, typename B>
    explicit Graph(A &&v, B &&dep)
      : nodes(std::forward<A>(v))
      , neighbours(std::forward<B>(dep))
    {
      for (Node_Id_Type i = 0; i < nodes.size(); ++i)
        {
          nodes[i].set_id(i);
        }
    }


    /// Get the nodes collection
    /// \return The vector of nodes
    auto
    get_nodes() const -> Node_Collection_Type const &
    {
      return nodes;
    }


    /// Get the dependencies (reference)
    /// \return The dependencies (reference)
    [[nodiscard]] auto
    get_neighbors_ref() -> Neighbours_Type &
    {
      return neighbours;
    }

    /// Get input nodes
    [[nodiscard]] auto
    get_input_nodes(Node_Id_Type id) const -> Neighbours_Type::value_type::first_type const &
    {
      return neighbours[id].first;
    }

    /// Get input nodes
    [[nodiscard]] auto
    get_output_nodes(Node_Id_Type id) const -> Neighbours_Type::value_type::second_type const &
    {
      return neighbours[id].second;
    }

    /// Checks if the given edge exists
    /// \param edge The edge
    /// \return True if the edge exists, false otherwise
    [[nodiscard]] auto
    check_edge(Edge_Type const &edge) const -> bool
    {
      return get_output_nodes(edge.first).contains(edge.second);
    }


    /// Get the number of nodes
    /// \return The number of nodes
    [[nodiscard]] auto
    size() const -> std::size_t
    {
      return nodes.size();
    }


    /// Checks if the graph is empty
    /// \return True if there are no stored nodes
    [[nodiscard]] auto
    empty() const -> bool
    {
      return nodes.empty();
    }


    /// Get the node with the given id
    /// \param id The id of the node
    /// \return The node
    auto
    operator[](std::size_t const &id) const -> Node_Type const &
    {
      return nodes[id];
    }


    /// Get the begin iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    begin()
    {
      return nodes.begin();
    }


    /// Get the end iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    end()
    {
      return nodes.end();
    }


    /// Get the begin iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    begin() const
    {
      return nodes.begin();
    }


    /// Get the end iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    end() const
    {
      return nodes.end();
    }


    /// Get the begin iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    cbegin() const
    {
      return nodes.cbegin();
    }


    /// Get the end iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    cend() const
    {
      return nodes.cend();
    }


    /// It deletes the nodes and dependencies of the graph
    void
    clear()
    {
      nodes.clear();
      neighbours.clear();
    }


    virtual ~Graph() = default;

  protected:
    /// Vector of all the nodes
    Node_Collection_Type nodes;

    /// Vector that contains all the neighbours of every node (first input, then output)
    Neighbours_Type neighbours;
  };


  /// Just another graph class...
  /// \tparam T Type of the content of the nodes
  template <typename T>
  class Graph<CNode<Content<T>>>
  {
  public:
    using Neighbours_Type      = std::vector<std::pair<Node_Id_Collection_Type, Node_Id_Collection_Type>>;
    using Node_Type            = CNode<Content<T>>;
    using Node_Collection_Type = std::vector<Node_Type>;

    Graph() = default;

    /// Construct a new Graph object
    /// \param v The collection of nodes ordered in an ascending order based on the id. To work with butcher, the
    /// nodes must be sorted in topological order, according to the Onnx IR specifications.
    /// \param dep Node neighbours (input and outputs of every node)
    template <typename A, typename B>
    explicit Graph(A &&v, B &&dep)
      : nodes(std::forward<A>(v))
      , neighbours(std::forward<B>(dep))
    {
      for (Node_Id_Type i = 0; i < nodes.size(); ++i)
        {
          nodes[i].set_id(i);
        }
    }

    explicit Graph(Node_Collection_Type const &v)
      : nodes(v)
    {
      for (Node_Id_Type i = 0; i < nodes.size(); ++i)
        {
          nodes[i].set_id(i);
        }

      compute_dependencies();
    }

    explicit Graph(Node_Collection_Type &&v)
      : nodes(std::move(v))
    {
      for (Node_Id_Type i = 0; i < this->nodes.size(); ++i)
        {
          this->nodes[i].set_id(i);
        }

      compute_dependencies();
    }


    /// Get the nodes collection
    /// \return The vector of nodes
    auto
    get_nodes() const -> Node_Collection_Type const &
    {
      return nodes;
    }


    /// Get the dependencies (reference)
    /// \return The dependencies (reference)
    [[nodiscard]] auto
    get_neighbors_ref() -> Neighbours_Type &
    {
      return neighbours;
    }

    /// Get input nodes
    [[nodiscard]] auto
    get_input_nodes(Node_Id_Type id) const -> Neighbours_Type::value_type::first_type const &
    {
      return neighbours[id].first;
    }

    /// Get input nodes
    [[nodiscard]] auto
    get_output_nodes(Node_Id_Type id) const -> Neighbours_Type::value_type::second_type const &
    {
      return neighbours[id].second;
    }

    /// Checks if the given edge exists
    /// \param edge The edge
    /// \return True if the edge exists, false otherwise
    [[nodiscard]] auto
    check_edge(Edge_Type const &edge) const -> bool
    {
      return get_output_nodes(edge.first).contains(edge.second);
    }


    /// Get the number of nodes
    /// \return The number of nodes
    [[nodiscard]] auto
    size() const -> std::size_t
    {
      return nodes.size();
    }


    /// Checks if the graph is empty
    /// \return True if there are no stored nodes
    [[nodiscard]] auto
    empty() const -> bool
    {
      return nodes.empty();
    }


    /// Get the node with the given id
    /// \param id The id of the node
    /// \return The node
    auto
    operator[](std::size_t const &id) const -> Node_Type const &
    {
      return nodes[id];
    }


    /// Get the begin iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    begin()
    {
      return nodes.begin();
    }


    /// Get the end iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    end()
    {
      return nodes.end();
    }


    /// Get the begin iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    begin() const
    {
      return nodes.begin();
    }


    /// Get the end iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    end() const
    {
      return nodes.end();
    }


    /// Get the begin iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    cbegin() const
    {
      return nodes.cbegin();
    }


    /// Get the end iterator of the nodes collection
    /// \return The iterator
    [[nodiscard]] auto
    cend() const
    {
      return nodes.cend();
    }


    /// It deletes the nodes and dependencies of the graph
    void
    clear()
    {
      nodes.clear();
      neighbours.clear();
    }


    virtual ~Graph() = default;

  protected:
    void
    compute_dependencies();

    /// Vector of all the nodes
    Node_Collection_Type nodes;

    /// Vector that contains all the neighbours of every node (first input, then output)
    Neighbours_Type neighbours;
  };

  template <typename T>
  void
  Graph<CNode<Content<T>>>::compute_dependencies()
  {
    // Reset the dependency vector.
    neighbours = Neighbours_Type();
    neighbours.resize(nodes.size());

    // Compute appearances of inputs/outputs for a node
    std::unordered_map<std::string, Node_Id_Collection_Type> input_appearances;
    std::unordered_map<std::string, Node_Id_Collection_Type> output_appearances;

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
          neighbours[node_id].first.insert(neib.cbegin(), neib.cend());
        for (auto node_id : neib)
          neighbours[node_id].second.insert(appearance.second.cbegin(), appearance.second.cend());
      }
  }
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_GRAPH_H
