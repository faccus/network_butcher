//
// Created by faccus on 25/08/21.
//

#ifndef NETWORK_BUTCHER_TYPE_INFO_H
#define NETWORK_BUTCHER_TYPE_INFO_H

#include <string>
#include "../Traits/Basic_traits.h"

/// Generic type contained in a onnx model (only type info, no values are actually stored)
class Type_info
{
protected:
  /// Name of the type
  std::string name;
public:

  /// Get the name of the type
  /// \return
  std::string get_name() {
    return name;
  };

  Type_info() = default;

  /// Virtual method to compute the total memory of the type
  /// \return Memory usage of the associated type
  virtual memory_type
  compute_memory_usage() const = 0;

  /// Default deconstructor
  virtual ~Type_info() = default;
};


#endif // NETWORK_BUTCHER_TYPE_INFO_H
