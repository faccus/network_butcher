//
// Created by faccus on 28/08/21
//

#ifndef NETWORK_BUTCHER_DENSE_TENSOR_H
#define NETWORK_BUTCHER_DENSE_TENSOR_H

#include "type_info.h"
#include "variant_attribute.h"

#include "utilities.h"

#include <vector>

namespace network_butcher::types
{
  /// tensor type of an onnx model
  class Dense_tensor : public Type_Info
  {
  private:
    /// Memory usage of the element type of the tensor
    Memory_Type type_tensor_memory;

    /// Shape of the tensor
    std::vector<Onnx_Element_Shape_Type> shape;

  public:
    /// Construct the tensor from the type id and the shape
    /// \param in_type_id onnx::TensorProto_DataType_* id
    /// \param in_shape Shape of the tensor
    /// \param given Is it already initialized?
    /// \param constant Is it constant?
    Dense_tensor(Type_Info_Id_Type       in_type_id,
                 std::vector<Onnx_Element_Shape_Type> in_shape,
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
    [[nodiscard]] Memory_Type
    compute_memory_usage() const override;


    /// Compute the number of elements in the tensor
    /// \return The number of elements in the tensor
    [[nodiscard]] Onnx_Element_Shape_Type
    compute_shape_volume() const override;


    /// Basic getter for shape
    /// \return The shape
    [[nodiscard]] inline std::vector<Onnx_Element_Shape_Type> const &
    get_shape() const override
    {
      return shape;
    }
  };
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_DENSE_TENSOR_H