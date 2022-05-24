//
// Created by faccus on 25/08/21.
//

#ifndef NETWORK_BUTCHER_TYPE_INFO_H
#define NETWORK_BUTCHER_TYPE_INFO_H

#include <string>
#include <vector>

#include "../Traits/Hardware_traits.h"
#include "../Traits/Type_info_traits.h"

/// Generic type contained in a onnx model (only type info, no values are
/// actually stored)
class Type_info
{
protected:
  /// Name of the type
  std::string name;

  /// Is the value of this type given by the network?
  bool t_initialized;
  bool constant;

public:
  /// Get the name of the type
  /// \return
  std::string
  get_name()
  {
    return name;
  }

  /// Get if the value of this type is given by the network
  /// \return True if it has been already initialized
  bool
  initialized()
  {
    return t_initialized;
  }

  void
  set_initialized(bool const &val)
  {
    t_initialized = val;
  }

  Type_info() = default;

  /// Virtual method to compute the total memory of the type
  /// \return Memory usage of the associated type
  virtual memory_type
  compute_memory_usage() const = 0;

  /// Basic getter for shape
  /// \return The shape
  virtual std::vector<shape_type> const &
  get_shape() const = 0;

  /// Default deconstructor
  virtual ~Type_info() = default;
};

#endif // NETWORK_BUTCHER_TYPE_INFO_H