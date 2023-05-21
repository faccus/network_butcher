//
// Created by faccus on 17/02/22.
//

#ifndef NETWORK_BUTCHER_CONTENT_H
#define NETWORK_BUTCHER_CONTENT_H

#include "Dense_tensor.h"
#include "DynamicType.h"


namespace network_butcher::types
{
  /// A simple class to store the content of an onnx layer
  /// \tparam T The tensor type
  template <class T>
  class Content
  {
  public:
    using io_collection        = io_collection_type<T>;
    using attribute_collection = std::unordered_map<std::string, DynamicType>;

  private:
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

  public:
    /// Generic make content class (only usable if T has default constructor)
    /// \tparam Arg The parameters
    /// \param arg The input parameters (if some of the fields are not provided, they are default initialized)
    /// \return The Content<T>
    template <typename... Arg>
    static Content<T>
    make_content(Arg &&...arg)
    {
      return Content<T>(std::forward<Arg>(arg)...);
    }

    /// Generic make content class (only usable if T has default constructor)
    template <typename A = io_collection,
              typename B = io_collection,
              typename C = io_collection,
              typename D = attribute_collection,
              typename E = std::string>
    explicit Content(A &&in = A(), B &&out = B(), C &&params = C(), D &&in_attributes = D(), E &&operation_name = "")
      : input(std::forward<A>(in))
      , output(std::forward<B>(out))
      , parameters(std::forward<C>(params))
      , attributes(std::forward<D>(in_attributes))
      , operation_id(std::forward<E>(operation_name))
    {}


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
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_CONTENT_H
