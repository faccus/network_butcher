//
// Created by faccus on 28/08/21
//

#ifndef NETWORK_BUTCHER_DENSE_TENSOR_H
#define NETWORK_BUTCHER_DENSE_TENSOR_H

#include "Type_info.h"
#include "../Utilities.h"
#include <vector>

/// Dense tensor type of an onnx model
class Dense_tensor : public Type_info
{
private:
  /// (onnx::TensorProto_DataType_*) Type id for the variables stored in the tensor
  int type_id = -1; // Maybe, it can be changed to the dimension of the type

  /// Shape of the tensor
  std::vector<long> shape;

public:
  /// Construct the tensor from the type id and the shape
  /// \param in_type_id onnx::TensorProto_DataType_* id
  /// \param in_shape Shape of the tensor
  Dense_tensor(int in_type_id, std::vector<long> in_shape);

  /// Construct the tensor from a onnx::ValueInfoProto object
  /// \param info onnx::ValueInfoProto object
  Dense_tensor(const onnx::ValueInfoProto &info);

  /// Compute the total memory of the type
  /// \return Memory usage of the associated type
  std::size_t
  compute_memory_usage() const override;
};

#endif // NETWORK_BUTCHER_DENSE_TENSOR_H
/*
 *
 * */