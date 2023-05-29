//
// Created by faccus on 28/08/21
//

#ifndef NETWORK_BUTCHER_DENSE_TENSOR_H
#define NETWORK_BUTCHER_DENSE_TENSOR_H

#include "DynamicType.h"
#include "Type_info.h"

#include "Utilities.h"

#include <vector>

namespace network_butcher::types
{
  /// tensor type of an onnx model
  class Dense_tensor : public Type_info
  {
  private:
    /// Memory usage of the element type of the tensor
    memory_type type_tensor_memory;

    /// Shape of the tensor
    std::vector<shape_type> shape;

  public:
    /// Construct the tensor from the type id and the shape
    /// \param in_type_id onnx::TensorProto_DataType_* id
    /// \param in_shape Shape of the tensor
    /// \param given Is it already initialized?
    /// \param constant Is it constant?
    Dense_tensor(type_info_id_type       in_type_id,
                 std::vector<shape_type> in_shape,
                 bool                    given    = false,
                 bool                    constant = false);


    /// Construct the tensor from a onnx::ValueInfoProto object
    /// \param info onnx::ValueInfoProto object
    /// \param given Is it already initialized?
    /// \param constant Is it constant?
    explicit Dense_tensor(const onnx::ValueInfoProto &info, bool given = false, bool constant = false);


    /// Construct the tensor from a onnx::TensorProto object
    /// \param info onnx::ValueInfoProto object
    /// \param given Is it already initialized?
    /// \param constant Is it constant?
    explicit Dense_tensor(const onnx::TensorProto &info, bool given = false, bool constant = false);


    /// Compute the total memory of the type
    /// \return Memory usage of the associated type
    [[nodiscard]] memory_type
    compute_memory_usage() const override;


    /// Compute the number of elements in the tensor
    /// \return The number of elements in the tensor
    [[nodiscard]] shape_type
    compute_shape_volume() const override;


    /// Basic getter for shape
    /// \return The shape
    [[nodiscard]] inline std::vector<shape_type> const &
    get_shape() const override
    {
      return shape;
    }
  };
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_DENSE_TENSOR_H