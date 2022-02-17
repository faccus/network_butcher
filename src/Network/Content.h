//
// Created by faccus on 17/02/22.
//

#ifndef NETWORK_BUTCHER_CONTENT_H
#define NETWORK_BUTCHER_CONTENT_H

#include "../Helpers/Traits/Basic_traits.h"
#include "../Helpers/Traits/Node_traits.h"
#include "../Helpers/Types/Dense_tensor.h"

class Content
{
private:
  /// Collection of the ids of inputs of the node
  io_id_collection_type input;
  /// Collection of the ids of outputs of the node
  io_id_collection_type output;
  /// Collection of the ids of parameters of the node
  io_id_collection_type parameters;
  /// Collection of the attributes of the node
  std::unordered_map<std::string, std::vector<long>> attributes;


  inline void
  set_attribute(std::string const &name, std::vector<long> tensor)
  {
    attributes.emplace(name, tensor);
  }

public:
  Content(io_id_collection_type in     = {},
          io_id_collection_type out    = {},
          io_id_collection_type params = {})
    : input(std::move(in))
    , output(std::move(out))
    , parameters(std::move(params))
  {}

  /// Read-only getter for input
  /// \return Const reference to input
  inline const io_id_collection_type &
  get_input() const
  {
    return input;
  }

  /// Read-only getter for output
  /// \return Const reference to output
  inline const io_id_collection_type &
  get_output() const
  {
    return output;
  }

  /// Read-only getter for parameters
  /// \return Const reference to parameters
  inline const io_id_collection_type &
  get_parameters() const
  {
    return parameters;
  }
};


#endif // NETWORK_BUTCHER_CONTENT_H
