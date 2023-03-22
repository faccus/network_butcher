//
// Created by faccus on 17/02/22.
//

#ifndef NETWORK_BUTCHER_CONTENT_H
#define NETWORK_BUTCHER_CONTENT_H

#include "Dense_tensor.h"
#include "DynamicType.h"


namespace network_butcher_types
{
  /// @brief A simple class to store the content of an onnx layer
  /// @tparam T The tensor type
  template <class T>
  class Content
  {
  public:
    using io_collection = io_collection_type<T>;

  private:
    /// @brief Collection of the ids of inputs of the node
    io_collection input;

    /// @brief Collection of the ids of outputs of the node
    io_collection output;

    /// @brief Collection of the ids of parameters of the node
    io_collection parameters;

    /// @brief Collection of the attributes of the node
    std::unordered_map<std::string, DynamicType> attributes;

    /// @brief The operation id (name)
    std::string operation_id;

  public:
    // Create a generic make_content function that takes as an input an arbitrary number of arguments and returns a
    // Content<T> object

    /// @brief Generic make content class (only usable if T has default constructor)
    /// @tparam Arg The parameters
    /// @param arg The input parameters (if some of the fields are not provided, they are default initialized)
    /// @return The Content<T>
    template <typename... Arg>
    static Content<T>
    make_content(Arg &&...arg)
    {
      return Content<T>(std::forward<Arg>(arg)...);
    }

    /// @brief Generic make content class (only usable if T has default constructor)
    Content(
      io_collection                                &&in             = io_collection(),
      io_collection                                &&out            = io_collection(),
      io_collection                                &&params         = io_collection(),
      std::unordered_map<std::string, DynamicType> &&in_attributes  = std::unordered_map<std::string, DynamicType>(),
      std::string                                  &&operation_name = "")
      : input(std::forward<io_collection>(in))
      , output(std::forward<io_collection>(out))
      , parameters(std::forward<io_collection>(params))
      , attributes(std::forward<std::unordered_map<std::string, DynamicType>>(in_attributes))
      , operation_id(std::forward<std::string>(operation_name))
    {}


    /// @brief Read-only getter for input
    /// @return Const reference to input
    inline const io_collection &
    get_input() const
    {
      return input;
    }


    /// @brief Read-only getter for output
    /// @return Const reference to output
    inline const io_collection &
    get_output() const
    {
      return output;
    }


    /// @brief Read-only getter for parameters
    /// @return Const reference to parameters
    inline const io_collection &
    get_parameters() const
    {
      return parameters;
    }


    /// @brief Read-only getter for attributes
    /// @return Const reference to attributes
    inline const std::unordered_map<std::string, DynamicType> &
    get_attributes() const
    {
      return attributes;
    }


    /// @brief Read-only getter for operation id
    /// @return Const reference to operation id
    inline const std::string &
    get_operation_id() const
    {
      return operation_id;
    }
  };
} // namespace network_butcher_types

#endif // NETWORK_BUTCHER_CONTENT_H
