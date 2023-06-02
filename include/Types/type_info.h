//
// Created by faccus on 25/08/21.
//

#ifndef NETWORK_BUTCHER_TYPE_INFO_H
#define NETWORK_BUTCHER_TYPE_INFO_H

#include <string>
#include <vector>

#include "starting_traits.h"

namespace network_butcher::types
{
  /// Generic type contained in a onnx model (only type info, no values are actually stored)
  class Type_info
  {
  protected:
    /// Name of the type
    std::string name;

    /// Is the value of this type given by the network?
    bool initialized{false};

    /// Is it constant?
    bool constant{false};

  public:
    Type_info() = default;

    Type_info(bool initialized, bool constant, std::string const &in_name = "")
      : initialized(initialized)
      , constant(constant)
      , name(in_name)
    {}


    /// Get the name of the type
    /// \return The name
    [[nodiscard]] std::string
    get_name() const
    {
      return name;
    }


    /// Get if the value of this type is given by the network
    /// \return True if it has been already initialized
    [[nodiscard]] bool
    is_initialized() const
    {
      return initialized;
    }


    /// Virtual method to compute the total memory of the type
    /// \return Memory usage of the associated type
    [[nodiscard]] virtual memory_type
    compute_memory_usage() const = 0;


    /// Basic getter for shape
    /// \return The shape
    [[nodiscard]] virtual std::vector<shape_type> const &
    get_shape() const = 0;


    /// Compute the number of elements in the tensor
    /// \return The number of elements in the tensor
    [[nodiscard]] virtual shape_type
    compute_shape_volume() const = 0;


    /// Default deconstructor
    virtual ~Type_info() = default;
  };
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_TYPE_INFO_H
