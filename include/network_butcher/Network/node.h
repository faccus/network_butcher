#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

#include <network_butcher/Types/dense_tensor.h>
#include <network_butcher/Traits/traits.h>

#include <memory>
#include <utility>

namespace network_butcher::types
{
  /// Just another node class...
  class Node
  {
  private:
    /// Current node id
    Node_Id_Type id;

  public:
    /// Name of the node
    std::string name;

    /// Basic constructor
    Node()
      : id(std::numeric_limits<Node_Id_Type>::max())
    {}

    /// Copy constructor
    /// \param other The node to copy
    Node(Node const &other) = default;

    /// Move constructor
    /// \param other The node to move
    Node(Node &&other) noexcept = default;

    /// Copy assignment operator
    /// \param other The node to copy
    /// \return The current node
    auto operator=(Node const &other) -> Node & = default;

    /// Move assignment operator
    /// \param other The node to move
    /// \return The current node
    auto operator=(Node &&other) noexcept -> Node & = default;


    /// Getter for the node id
    /// \return The node id
    [[nodiscard]] auto
    get_id() const -> Node_Id_Type
    {
      return id;
    }

    /// Setter for the node id
    /// \param starting_id The new id
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
    /// Alias for the content type
    using Content_Type = T;

    /// Node content
    Content_Type content;

    /// Default constructor
    CNode() = default;

    /// Copy constructor
    /// \param other The node to copy
    CNode(CNode const &other) = default;

    /// Move constructor
    /// \param other The node to move
    CNode(CNode &&other) noexcept = default;

    /// Copy assignment operator
    /// \param other The node to copy
    /// \return The current node
    auto operator=(CNode const &other) -> CNode & = default;

    /// Move assignment operator
    /// \param other The node to move
    /// \return The current node
    auto operator=(CNode &&other) noexcept -> CNode & = default;

    /// Basic constructor for a node, moving a content
    explicit CNode(Content_Type &&starting_content)
      : Node()
      , content(std::move(starting_content))
    {}

    /// Basic constructor for a node
    explicit CNode(Content_Type const &starting_content)
      : Node()
      , content(starting_content)
    {}

    ~CNode() override = default;
  };
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_NODE_H
