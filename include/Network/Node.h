//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

#include "../Helpers/Traits/Basic_traits.h"
#include "../Helpers/Types/Dense_tensor.h"

#include <utility>
#include <memory>

namespace network_butcher_types
{
  template <class T>
  class Graph;

  /// A node of a graph
  template <class T>
  class Node
  {
  private:
    friend class Graph<T>;

    /// Current node id
    node_id_type id;


    /// Basic constructor for a node
    /// \param starting_id Initial node id
    /// \param starting_content Initial content
    Node(node_id_type starting_id, T starting_content)
      : id(starting_id)
      , content(std::move(starting_content))
    {}


  public:
    /// Node content
    T content;


    /// Basic move constructor for a node
    explicit Node(T &&starting_content)
      : id(std::numeric_limits<node_id_type>::max())
      , content(std::move(starting_content))
    {}

    /// Basic move constructor for a node
    explicit Node(T const &starting_content)
      : id(std::numeric_limits<node_id_type>::max())
      , content(starting_content)
    {}


    /// Getter for the node id
    inline node_id_type
    get_id() const
    {
      return id;
    }
  };
}

#endif // NETWORK_BUTCHER_NODE_H
