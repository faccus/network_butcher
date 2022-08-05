//
// Created by faccus on 28/08/21
//

#ifndef NETWORK_BUTCHER_DENSE_TENSOR_H
#define NETWORK_BUTCHER_DENSE_TENSOR_H

#include "../Traits/Type_info_traits.h"
#include "../Utilities.h"
#include "DynamicType.h"
#include "Type_info.h"

#include <vector>

namespace network_butcher_types
{
  /// Dense tensor type of an onnx model
  class Dense_tensor : public Type_info
  {
  private:
    /// (onnx::TensorProto_DataType_*) Type id for the variables stored in the
    /// tensor
    type_info_id_type type_id = -1; // Maybe, it can be changed to the dimension of the type

    /// Shape of the tensor
    std::vector<shape_type> shape;

    DynamicType value;

  public:
    /// Construct the tensor from the type id and the shape
    /// \param in_type_id onnx::TensorProto_DataType_* id
    /// \param in_shape Shape of the tensor
    Dense_tensor(type_info_id_type       in_type_id,
                 std::vector<shape_type> in_shape,
                 bool                    given    = false,
                 bool                    constant = false);

    /// Construct the tensor from a onnx::ValueInfoProto object
    /// \param info onnx::ValueInfoProto object
    Dense_tensor(const onnx::ValueInfoProto &info, bool given = false, bool constant = false);

    /// Construct the tensor from a onnx::TensorProto object
    /// \param info onnx::ValueInfoProto object
    Dense_tensor(const onnx::TensorProto &info, bool given = false, bool constant = false);

    /// Compute the total memory of the type
    /// \return Memory usage of the associated type
    memory_type
    compute_memory_usage() const override;

    /// Basic getter for shape
    /// \return The shape
    inline std::vector<shape_type> const &
    get_shape() const override
    {
      return shape;
    }

    /// Basic getter for the value of the tensor
    /// \return The tensor
    inline DynamicType const &
    get_value() const
    {
      return value;
    }

    /// Basic setter for the value of the tensor
    /// \param value The value to be setted
    inline void
    set_value(DynamicType const &value)
    {
      this->value = value;
    }
  };
} // namespace network_butcher_types

#endif // NETWORK_BUTCHER_DENSE_TENSOR_H