//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

#include "basic_traits.h"
#include "dense_tensor.h"

#include <memory>
#include <utility>

namespace network_butcher::types
{
  /// Just another node class...
  /// \tparam T Type of the content of the node
  class Node
  {
  private:
    /// Current node id
    Node_Id_Type id;

  public:
    /// Name of the node
    std::string name;

    /// Basic move constructor for a node
    Node()
      : id(std::numeric_limits<Node_Id_Type>::max())
    {}


    /// Getter for the node id
    [[nodiscard]] inline Node_Id_Type
    get_id() const
    {
      return id;
    }

    void
    set_id(Node_Id_Type starting_id)
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

    /// Node content
    Content_Type content;

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
