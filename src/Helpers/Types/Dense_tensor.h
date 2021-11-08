//
// Created by faccus on 28/08/21
//

#ifndef NETWORK_BUTCHER_DENSE_TENSOR_H
#define NETWORK_BUTCHER_DENSE_TENSOR_H

#include "Type_info.h"
#include "../Utilities.h"
#include "../Traits/Type_info_traits.h"

#include <vector>

/// Dense tensor type of an onnx model
class Dense_tensor : public Type_info
{
private:
  /// (onnx::TensorProto_DataType_*) Type id for the variables stored in the tensor
  type_info_id_type type_id = -1; // Maybe, it can be changed to the dimension of the type

  /// Shape of the tensor
  std::vector<shape_type> shape;

public:
  /// Construct the tensor from the type id and the shape
  /// \param in_type_id onnx::TensorProto_DataType_* id
  /// \param in_shape Shape of the tensor
  Dense_tensor(type_info_id_type in_type_id, std::vector<shape_type> in_shape);

  /// Construct the tensor from a onnx::ValueInfoProto object
  /// \param info onnx::ValueInfoProto object
  Dense_tensor(const onnx::ValueInfoProto &info);

  /// Compute the total memory of the type
  /// \return Memory usage of the associated type
  memory_type
  compute_memory_usage() const override;

  /// Basic getter for shape
  /// \return The shape
  std::vector<shape_type> const &
  get_shape() const override
  {
    return shape;
  }
};

#endif // NETWORK_BUTCHER_DENSE_TENSOR_H
/*
 *
 * */