//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

#include "Basic_traits.h"
#include "Dense_tensor.h"

#include <memory>
#include <utility>

namespace network_butcher {

namespace types
{
  template <class T>
  class Graph;

  /// Just another node class...
  /// \tparam T Type of the content of the node
  template <class T>
  class Node
  {
  public:
    using Content_Type = T;    
  
  private:
    friend class Graph<Content_Type>;

    /// Current node id
    node_id_type id;


    /// Basic constructor for a node
    /// \param starting_id Initial node id
    /// \param starting_content Initial content
    Node(node_id_type starting_id, Content_Type starting_content)
      : id(starting_id)
      , content(std::move(starting_content))
    {}


  public:
  
    /// Node content
    Content_Type content;

    /// Name of the node
    std::string name;


    /// Basic move constructor for a node
    explicit Node(Content_Type &&starting_content)
      : id(std::numeric_limits<node_id_type>::max())
      , content(std::move(starting_content))
    {}

    /// Basic move constructor for a node
    explicit Node(Content_Type const &starting_content)
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
} // namespace network_butcher_types

}

#endif // NETWORK_BUTCHER_NODE_H
