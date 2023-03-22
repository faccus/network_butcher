//
// Created by faccus on 25/08/21.
//

#ifndef NETWORK_BUTCHER_TYPE_INFO_H
#define NETWORK_BUTCHER_TYPE_INFO_H

#include <string>
#include <vector>

#include "Type_info_traits.h"

namespace network_butcher
{

  namespace types
  {
    /// @brief Generic type contained in a onnx model (only type info, no values are actually stored)
    class Type_info
    {
    protected:
      /// @brief Name of the type
      std::string name;

      /// @brief Is the value of this type given by the network?
      bool t_initialized;

      /// @brief Is it constant?
      bool constant;

    public:
      Type_info() = default;

      Type_info(bool initialized, bool constant, std::string in_name = "")
        : t_initialized(initialized)
        , constant(constant)
        , name(in_name)
      {}


      /// @brief Get the name of the type
      /// @return The name
      std::string
      get_name()
      {
        return name;
      }


      /// @brief Get if the value of this type is given by the network
      /// @return True if it has been already initialized
      bool
      initialized()
      {
        return t_initialized;
      }


      /// @brief Virtual method to compute the total memory of the type
      /// @return Memory usage of the associated type
      virtual memory_type
      compute_memory_usage() const = 0;


      /// @brief Basic getter for shape
      /// @return The shape
      virtual std::vector<shape_type> const &
      get_shape() const = 0;


      /// @brief Compute the number of elements in the tensor
      /// @return The number of elements in the tensor
      virtual shape_type
      compute_shape_volume() const = 0;


      /// @brief Default deconstructor
      virtual ~Type_info() = default;
    };
  } // namespace types

} // namespace network_butcher

#endif // NETWORK_BUTCHER_TYPE_INFO_H
