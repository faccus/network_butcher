//
// Created by faccus on 17/02/22.
//

#ifndef NETWORK_BUTCHER_CONTENT_H
#define NETWORK_BUTCHER_CONTENT_H

#include "Dense_tensor.h"
#include "DynamicType.h"


namespace network_butcher_types
{
  template <class T>
  class Content
  {
  private:
    /// Collection of the ids of inputs of the node
    io_collection_type<T> input;
    /// Collection of the ids of outputs of the node
    io_collection_type<T> output;
    /// Collection of the ids of parameters of the node
    io_collection_type<T> parameters;
    /// Collection of the attributes of the node
    std::unordered_map<std::string, std::unique_ptr<DynamicType_Base>> attributes;
    /// Operation name
    std::string operation_id;

    inline void
    set_attribute(std::string const &name, std::vector<std::size_t> const &tensor)
    {
      attributes.emplace(name, tensor);
    }


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


    template <class A = io_collection_type<T>,
              class B = io_collection_type<T>,
              class C = io_collection_type<T>,
              class D = std::unordered_map<std::string, std::vector<DynamicType>>,
              class E = std::string>
    Content(A &&in = A(), B &&out = B(), C &&params = C(), D &&in_attributes = D(), E &&operation_name = E())
      : input(std::forward<A>(in))
      , output(std::forward<B>(out))
      , parameters(std::forward<C>(params))
      , attributes(std::forward<D>(in_attributes))
      , operation_id(std::forward<E>(operation_name))
    {}

    /// Read-only getter for input
    /// \return Const reference to input
    inline const io_collection_type<T> &
    get_input() const
    {
      return input;
    }

    /// Read-only getter for output
    /// \return Const reference to output
    inline const io_collection_type<T> &
    get_output() const
    {
      return output;
    }

    /// Read-only getter for parameters
    /// \return Const reference to parameters
    inline const io_collection_type<T> &
    get_parameters() const
    {
      return parameters;
    }

    /// Read-only getter for attributes
    /// \return Const reference to the attributes
    inline const std::unordered_map<std::string, std::vector<DynamicType>> &
    get_attributes() const
    {
      return attributes;
    }

    /// Read-only getter for the operation id
    /// \return Operation id
    inline const std::string &
    get_operation_id() const
    {
      return operation_id;
    }
  };
} // namespace network_butcher_types

#endif // NETWORK_BUTCHER_CONTENT_H
