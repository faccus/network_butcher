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
  /// Generic type contained in a onnx model (only type information, no values are actually stored)
  class Type_Info
  {
  protected:
    /// Name of the type
    std::string name;

    /// Is the value of this type given by the network?
    bool initialized{false};

    /// Is it constant?
    bool constant{false};

  public:
    Type_Info() = default;

    Type_Info(bool initialized, bool constant, std::string const &in_name = "")
      : initialized(initialized)
      , constant(constant)
      , name(in_name)
    {}


    /// Get the name of the type
    /// \return The name
    [[nodiscard]] auto
    get_name() const -> std::string const &
    {
      return name;
    }


    /// Get if the value of this type is given by the network
    /// \return True if it has been already initialized
    [[nodiscard]] auto
    is_initialized() const -> bool
    {
      return initialized;
    }


    /// Virtual method to compute the total memory of the type
    /// \return Memory usage of the associated type
    [[nodiscard]] virtual auto
    compute_memory_usage() const -> Memory_Type = 0;


    /// Basic getter for shape
    /// \return The shape
    [[nodiscard]] virtual auto
    get_shape() const -> std::vector<Onnx_Element_Shape_Type> const & = 0;


    /// Compute the number of elements in the tensor
    /// \return The number of elements in the tensor
    [[nodiscard]] virtual auto
    compute_shape_volume() const -> Onnx_Element_Shape_Type = 0;


    /// Default deconstructor
    virtual ~Type_Info() = default;
  };
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_TYPE_INFO_H
