//
// Created by faccus on 17/02/22.
//

#ifndef NETWORK_BUTCHER_CONTENT_H
#define NETWORK_BUTCHER_CONTENT_H

#include "dense_tensor.h"
#include "variant_attribute.h"


namespace network_butcher::types
{
  template <typename T>
  class Content_Builder;

  /// A simple class to store the content of an onnx layer
  /// \tparam T The tensor type
  template <typename T>
  class Content
  {
  public:
    /// Inputs, Outputs and parameters are of this type
    using io_collection_type = Io_Collection_Type<T>;

    /// The attributes are of this type
    using attribute_collection_type = std::unordered_map<std::string, Variant_Attribute>;

    /// The type of the elements stored in io_collection (indexed by std::string)
    using element_type = T;

  private:
    friend class Content_Builder<T>;

    /// Collection of the ids of inputs of the node
    io_collection_type input;

    /// Collection of the ids of outputs of the node
    io_collection_type output;

    /// Collection of the ids of parameters of the node
    io_collection_type parameters;

    /// Collection of the attributes of the node
    attribute_collection_type attributes;

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
    inline const io_collection_type &
    get_input() const
    {
      return input;
    }


    /// Read-only getter for output
    /// \return Const reference to output
    inline const io_collection_type &
    get_output() const
    {
      return output;
    }


    /// Read-only getter for parameters
    /// \return Const reference to parameters
    inline const io_collection_type &
    get_parameters() const
    {
      return parameters;
    }


    /// Read-only getter for attributes
    /// \return Const reference to attributes
    inline const attribute_collection_type &
    get_attributes() const
    {
      return attributes;
    }


    /// Read-only getter for operation id
    /// \return Const reference to operation id
    inline const std::string &
    get_operation_id() const
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
    /// \return A reference to the builder
    template <typename A = decltype(Content_Type::input)>
    Content_Builder &
    set_input(A &&in)
    {
      res.input = std::forward<A>(in);
      return *this;
    }

    /// Add the specified input to Content (using perfect forwarding)
    /// \param out The output field of Content
    /// \return A reference to the builder
    template <typename A = decltype(Content_Type::output)>
    Content_Builder &
    set_output(A &&out)
    {
      res.output = std::forward<A>(out);
      return *this;
    }


    /// Add the specified input to Content (using perfect forwarding)
    /// \param params The parameters field of Content
    /// \return A reference to the builder
    template <typename A = decltype(Content_Type::parameters)>
    Content_Builder &
    set_parameters(A &&params)
    {
      res.parameters = std::forward<A>(params);
      return *this;
    }


    /// Add the specified input to Content (using perfect forwarding)
    /// \param attributes The attributes field of Content
    /// \return A reference to the builder
    template <typename A = decltype(Content_Type::attributes)>
    Content_Builder &
    set_attributes(A &&attributes)
    {
      res.attributes = std::forward<A>(attributes);
      return *this;
    }


    /// Add the specified input to Content (using perfect forwarding)
    /// \param operation_id The operation_id field of Content
    /// \return A reference to the builder
    template <typename A = decltype(Content_Type::operation_id)>
    Content_Builder &
    set_operation_id(A &&operation_id)
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
