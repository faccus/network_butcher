//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

#include "Basic_traits.h"
#include "Dense_tensor.h"

#include <memory>
#include <utility>

namespace network_butcher::types
{
  /// Just another node class...
  /// \tparam T Type of the content of the node
  class Node
  {
  protected:
    /// Current node id
    node_id_type id;

    /// Basic constructor for a node
    /// \param starting_id Initial node id
    /// \param starting_content Initial content
    explicit Node(node_id_type starting_id)
      : id(starting_id)
    {}

  public:
    /// Name of the node
    std::string name;

    /// Basic move constructor for a node
    Node()
      : id(std::numeric_limits<node_id_type>::max())
    {}


    /// Getter for the node id
    [[nodiscard]] inline node_id_type
    get_id() const
    {
      return id;
    }

    void
    set_id(node_id_type starting_id)
    {
      id = starting_id;
    }

    virtual ~Node() = default;
  };


  /// Just another node class...
  /// \tparam T Type of the content of the node
  template <class T>
  class CNode : public Node
  {
  public:
    using Content_Type = T;

  protected:
    /// Basic constructor for a node
    /// \param starting_id Initial node id
    /// \param starting_content Initial content
    CNode(node_id_type starting_id, Content_Type starting_content)
      : Node(starting_id)
      , content(std::move(starting_content))
    {}

  public:
    /// Node content
    Content_Type content;

    /// Name of the node
    std::string name;

    CNode() = default;

    /// Basic move constructor for a node
    explicit CNode(Content_Type &&starting_content)
      : Node()
      , content(std::move(starting_content))
    {}

    /// Basic move constructor for a node
    explicit CNode(Content_Type const &starting_content)
      : Node()
      , content(starting_content)
    {}

    ~CNode() override = default;
  };
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_NODE_H
