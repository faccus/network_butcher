//
// Created by faccus on 17/02/22.
//

#ifndef NETWORK_BUTCHER_CONTENT_H
#define NETWORK_BUTCHER_CONTENT_H

#include "../Traits/Basic_traits.h"
#include "Dense_tensor.h"
#include "DynamicType.h"

template <class T>
class Content
{
private:
  friend class IO_Manager;

  /// Collection of the ids of inputs of the node
  io_collection_type<T> input;
  /// Collection of the ids of outputs of the node
  io_collection_type<T> output;
  /// Collection of the ids of parameters of the node
  io_collection_type<T> parameters;
  /// Collection of the attributes of the node
  std::unordered_map<std::string, std::vector<DynamicType>> attributes;
  /// Operation name
  std::string operation_id;

  inline void
  set_attribute(std::string const &name, std::vector<std::size_t> const &tensor)
  {
    attributes.emplace(name, tensor);
  }

public:
  Content(io_collection_type<T> in     = {},
          io_collection_type<T> out    = {},
          io_collection_type<T> params = {},
          std::unordered_map<std::string, std::vector<DynamicType>>
                      in_attributes  = {},
          std::string operation_name = "")
    : input(std::move(in))
    , output(std::move(out))
    , parameters(std::move(params))
    , attributes(std::move(in_attributes))
    , operation_id(std::move(operation_name))
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

#endif // NETWORK_BUTCHER_CONTENT_H
