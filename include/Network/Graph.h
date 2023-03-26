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

#include "Content.h"
#include "Node.h"

#include "Graph_dev_traits.h"
#include "Node_traits.h"

namespace network_butcher
{
  namespace types
  {
    /// \brief Just another graph class...
    /// \tparam T Type of the content of the nodes
    template <typename T>
    class Graph
    {
    public:
      using Dependencies_Type    = network_butcher::types::Dependencies_Type;
      using Node_Type            = network_butcher::types::Node_Type<T>;
      using Node_Collection_Type = network_butcher::types::Node_Collection_Type<T>;
      using Node_Internal_Type   = T;


      Graph() = default;


      Graph(Graph const &) = default;
      Graph &
      operator=(Graph const &) = default;


      Graph(Graph &&) = default;

      Graph &
      operator=(Graph &&) = default;


      /// Construct a new Graph object
      /// \param v The collection of nodes ordered in an ascending order based on the id. To work with butcher, the nodes must be sorted in topological order, according to the Onnx IR specifications.
      /// \param dep Node dependencies (input and outputs of every node)
      template <typename A, typename B>
      explicit Graph(A &&v, B &&dep = {})
        : nodes(std::forward<A>(v))
        , dependencies(std::forward<B>(dep))
      {
        for (node_id_type i = 0; i < nodes.size(); ++i)
          nodes[i].id = i;
      }


      /// Get the nodes collection
      /// \return The vector of nodes
      inline const Node_Collection_Type &
      get_nodes() const
      {
        return nodes;
      }


      /// Get the nodes collection (reference)
      /// \return The vector of nodes (reference)
      inline Node_Collection_Type &
      get_nodes_ref() const
      {
        return nodes;
      }


      /// Get the dependencies
      /// \return The dependencies
      [[nodiscard]] inline const Dependencies_Type &
      get_neighbors() const
      {
        return dependencies;
      }


      /// Get the dependencies (reference)
      /// \return The dependencies (reference)
      [[nodiscard]] inline Dependencies_Type &
      get_neighbors_ref()
      {
        return dependencies;
      }


      /// Get the number of nodes
      /// \return The number of nodes
      [[nodiscard]] inline const std::size_t
      size() const
      {
        return nodes.size();
      }


      /// Checks if the graph is empty
      /// \return True if there are no stored nodes
      [[nodiscard]] inline const std::size_t
      empty() const
      {
        return nodes.empty();
      }


      /// Get the node with the given id
      /// \param id The id of the node
      /// \return The node
      Node_Type const &
      operator[](int id) const
      {
        return nodes[id];
      }


      /// Get the begin iterator of the nodes collection
      /// \return The iterator
      typename Node_Collection_Type::const_iterator
      cbegin() const
      {
        return nodes.cbegin();
      }


      /// Get the end iterator of the nodes collection
      /// \return The iterator
      typename Node_Collection_Type::const_iterator
      cend() const
      {
        return nodes.cend();
      }


      /// It remoces the node with the given id
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

      /// Vector that contains all the neighbours of every node (first input, then output)
      Dependencies_Type dependencies;
    };

    template <typename T>
    class Graph<Content<T>>
    {
    public:
      using Dependencies_Type    = network_butcher::types::Dependencies_Type;
      using Node_Type            = network_butcher::types::Node_Type<Content<T>>;
      using Node_Collection_Type = network_butcher::types::Node_Collection_Type<Content<T>>;
      using Node_Internal_Type   = T;
      using Node_Content_Type    = Content<T>;


      Graph() = default;

      Graph(Graph const &) = default;
      Graph &
      operator=(Graph const &) = default;

      Graph(Graph &&) = default;
      Graph &
      operator=(Graph &&) = default;


      /// Construct a new Graph object
      /// \param v The collection of nodes ordered in an ascending order based on the id. To work with butcher, the nodes must be sorted in topological order, according to the Onnx IR specifications.
      /// \param dep Node dependencies (input and outputs of every node)
      template <typename A, typename B>
      explicit Graph(A &&v, B &&dep)
        : nodes(std::forward<A>(v))
        , dependencies(std::forward<B>(dep))
      {
        for (node_id_type i = 0; i < nodes.size(); ++i)
          nodes[i].id = i;
      }


      /// Construct a new Graph object
      /// \param v The collection of nodes ordered in an ascending order based on the id. To work with butcher, the nodes must be sorted in topological order, according to the Onnx IR specifications.
      template <typename A>
      explicit Graph(A &&v)
        : nodes(std::forward<A>(v))
      {
        for (node_id_type i = 0; i < nodes.size(); ++i)
          nodes[i].id = i;

        compute_dependencies();
      }


      /// Get the nodes collection
      /// \return The vector of nodes
      inline const Node_Collection_Type &
      get_nodes() const
      {
        return nodes;
      }


      /// Get the nodes collection (reference)
      /// \return The vector of nodes (reference)
      inline Node_Collection_Type &
      get_nodes_ref() const
      {
        return nodes;
      }


      /// Get the dependencies
      /// \return The dependencies
      [[nodiscard]] inline const Dependencies_Type &
      get_neighbors() const
      {
        return dependencies;
      }


      /// Get the dependencies (reference)
      /// \return The dependencies (reference)
      [[nodiscard]] inline Dependencies_Type &
      get_neighbors_ref()
      {
        return dependencies;
      }


      /// Get the number of nodes
      /// \return The number of nodes
      [[nodiscard]] inline const std::size_t
      size() const
      {
        return nodes.size();
      }


      /// Checks if the graph is empty
      /// \return True if there are no stored nodes
      [[nodiscard]] inline const std::size_t
      empty() const
      {
        return nodes.empty();
      }


      /// Get the node with the given id
      /// \param id The id of the node
      /// \return The node
      Node_Type const &
      operator[](int id) const
      {
        return nodes[id];
      }


      /// Get the begin iterator of the nodes collection
      /// \return The iterator
      typename Node_Collection_Type::const_iterator
      cbegin() const
      {
        return nodes.cbegin();
      }


      /// Get the end iterator of the nodes collection
      /// \return The iterator
      typename Node_Collection_Type::const_iterator
      cend() const
      {
        return nodes.cend();
      }


      /// It removes the node with the given id
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
  } // namespace types

} // namespace network_butcher

#endif // NETWORK_BUTCHER_GRAPH_H
