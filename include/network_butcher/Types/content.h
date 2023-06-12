#ifndef NETWORK_BUTCHER_CONTENT_H
#define NETWORK_BUTCHER_CONTENT_H

#include <network_butcher/Types/dense_tensor.h>
#include <network_butcher/Types/variant_attribute.h>

namespace network_butcher::types
{
  template <typename T>
  class Content_Builder;

  /// A simple class to store the content of an onnx layer
  /// \tparam T The tensor type
  template <typename T>
  class Content
  {
  private:
    friend class Content_Builder<T>;

    /// Collection of the ids of inputs of the node
    Io_Collection_Type<T> input;

    /// Collection of the ids of outputs of the node
    Io_Collection_Type<T> output;

    /// Collection of the ids of parameters of the node
    Io_Collection_Type<T> parameters;

    /// Collection of the attributes (aka hyper-parameters) of the node
    std::unordered_map<std::string, Variant_Attribute> attributes;

    /// The operation id (name)
    std::string operation_id;

    Content() = default;

  public:
    /// Constructor of Content. It requires all the options. If they are not available, use the builder class
    /// \param in input
    /// \param out output
    /// \param params parameters
    /// \param attrs attributes
    /// \param op_id operation_id
    template <typename A, typename B, typename C, typename D, typename E>
    Content(A &&in, B &&out, C &&params, D &&attrs, E &&op_id)
      : input(std::forward<A>(in))
      , output(std::forward<B>(out))
      , parameters(std::forward<C>(params))
      , attributes(std::forward<D>(attrs))
      , operation_id(std::forward<E>(op_id))
    {}

    /// Read-only getter for input
    /// \return Const reference to input
    auto
    get_input() const -> Io_Collection_Type<T> const &
    {
      return input;
    }


    /// Read-only getter for output
    /// \return Const reference to output
    auto
    get_output() const -> Io_Collection_Type<T> const &
    {
      return output;
    }


    /// Read-only getter for parameters
    /// \return Const reference to parameters
    auto
    get_parameters() const -> Io_Collection_Type<T> const &
    {
      return parameters;
    }


    /// Read-only getter for attributes
    /// \return Const reference to attributes
    auto
    get_attributes() const -> std::unordered_map<std::string, Variant_Attribute> const &
    {
      return attributes;
    }


    /// Read-only getter for operation id
    /// \return Const reference to operation id
    auto
    get_operation_id() const -> std::string const &
    {
      return operation_id;
    }
  };


  /// Simple builder class for Content.
  /// \tparam T
  template <typename T>
  class Content_Builder
  {
  public:
    using Content_Type = Content<T>;

    /// Add the specified input to Content (using perfect forwarding)
    /// \param in The input field of Content
    /// \return Reference to the builder
    template <typename A = decltype(Content_Type::input)>
    auto
    set_input(A &&in) -> Content_Builder &
    {
      res.input = std::forward<A>(in);
      return *this;
    }

    /// Add the specified output to Content (using perfect forwarding)
    /// \param out The output field of Content
    /// \return Reference to the builder
    template <typename A = decltype(Content_Type::output)>
    auto
    set_output(A &&out) -> Content_Builder &
    {
      res.output = std::forward<A>(out);
      return *this;
    }


    /// Add the specified parameters to Content (using perfect forwarding)
    /// \param params The parameters field of Content
    /// \return Reference to the builder
    template <typename A = decltype(Content_Type::parameters)>
    auto
    set_parameters(A &&params) -> Content_Builder &
    {
      res.parameters = std::forward<A>(params);
      return *this;
    }


    /// Add the specified attributes to Content (using perfect forwarding)
    /// \param attributes The attributes field of Content
    /// \return Reference to the builder
    template <typename A = decltype(Content_Type::attributes)>
    auto
    set_attributes(A &&attributes) -> Content_Builder &
    {
      res.attributes = std::forward<A>(attributes);
      return *this;
    }


    /// Add the specified operation_id to Content (using perfect forwarding)
    /// \param operation_id The operation_id field of Content
    /// \return Reference to the builder
    template <typename A = decltype(Content_Type::operation_id)>
    auto
    set_operation_id(A &&operation_id) -> Content_Builder &
    {
      res.operation_id = std::forward<A>(operation_id);
      return *this;
    }

    /// Build the Content object. Notice that it can only be called if the builder is a rvalue.
    /// \return The moved content
    auto
    build() &&
    {
      return std::move(res);
    }

    /// Default constructor
    Content_Builder() = default;

    /// Copy constructor and copy assignment are deleted
    Content_Builder(const Content_Builder &) = delete;
    Content_Builder &
    operator=(const Content_Builder &) = delete;

    /// Move constructor and move assignment are deleted
    Content_Builder(Content_Builder &&) = delete;
    Content_Builder &
    operator=(Content_Builder &&) = delete;

    /// Destructor
    ~Content_Builder() = default;

  private:
    Content_Type res;
  };
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_CONTENT_H
