//
// Created by faccus on 17/02/22.
//

#ifndef NETWORK_BUTCHER_CONTENT_H
#define NETWORK_BUTCHER_CONTENT_H

#include "Dense_tensor.h"
#include "DynamicType.h"


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
    using io_collection        = io_collection_type<T>;
    using attribute_collection = std::unordered_map<std::string, DynamicType>;

  private:
    friend class Content_Builder<T>;

    /// Collection of the ids of inputs of the node
    io_collection input;

    /// Collection of the ids of outputs of the node
    io_collection output;

    /// Collection of the ids of parameters of the node
    io_collection parameters;

    /// Collection of the attributes of the node
    attribute_collection attributes;

    /// The operation id (name)
    std::string operation_id;

    Content() = default;

  public:
    /// Read-only getter for input
    /// \return Const reference to input
    inline const io_collection &
    get_input() const
    {
      return input;
    }


    /// Read-only getter for output
    /// \return Const reference to output
    inline const io_collection &
    get_output() const
    {
      return output;
    }


    /// Read-only getter for parameters
    /// \return Const reference to parameters
    inline const io_collection &
    get_parameters() const
    {
      return parameters;
    }


    /// Read-only getter for attributes
    /// \return Const reference to attributes
    inline const attribute_collection &
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


  template <typename T>
  class Content_Builder
  {
  public:
    using io_collection        = io_collection_type<T>;
    using attribute_collection = std::unordered_map<std::string, DynamicType>;

    template <typename A = decltype(Content<T>::input)>
    Content_Builder &
    set_input(A &&in)
      requires std::is_assignable_v<A, decltype(Content<T>::input)>
    {
      res.input = std::forward<A>(in);
      return *this;
    }

    template <typename A = decltype(Content<T>::output)>
    Content_Builder &
    set_output(A &&out)
      requires std::is_assignable_v<A, decltype(Content<T>::output)>
    {
      res.output = std::forward<A>(out);
      return *this;
    }

    template <typename A = decltype(Content<T>::parameters)>
    Content_Builder &
    set_parameters(A &&params)
      requires std::is_assignable_v<A, decltype(Content<T>::parameters)>
    {
      res.parameters = std::forward<A>(params);
      return *this;
    }

    template <typename A = decltype(Content<T>::attributes)>
    Content_Builder &
    set_attributes(A &&attributes)
      requires std::is_assignable_v<A, decltype(Content<T>::attributes)>
    {
      res.attributes = std::forward<A>(attributes);
      return *this;
    }

    template <typename A = decltype(Content<T>::operation_id)>
    Content_Builder &
    set_operation_id(A &&operation_id)
      requires std::is_assignable_v<A, decltype(Content<T>::operation_id)>
    {
      res.operation_id = std::forward<A>(operation_id);
      return *this;
    }

    auto
    build() &&
    {
      return std::move(res);
    }

    Content_Builder() = default;

  private:
    Content<T> res;
  };


} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_CONTENT_H
