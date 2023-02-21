//
// Created by faccus on 30/08/21.
//

#include "Dense_tensor.h"
#include <gtest/gtest.h>

namespace {
  using namespace network_butcher_types;

  TEST(DenseTensorTest, ConstuctorAndMemoryUsage)
  {
    Dense_tensor d(onnx::TensorProto_DataType_INT64, {1, 1, 2, 2}); // total memory 2*2*64=256 bits

    auto res = d.compute_memory_usage();

    ASSERT_EQ(res, 4 * sizeof(int64_t));
  }

  TEST(DenseTensorTest, ConstuctorValueInfoProto)
  {
    memory_type lhs = 0;

    {
      onnx::ValueInfoProto   value;
      onnx::TypeProto        typeProto;
      onnx::TypeProto_Tensor tensor;
      onnx::TensorShapeProto shapeProto;

      for (int i = 0; i < 4; ++i)
        {
          auto dim = shapeProto.add_dim();
          dim->set_dim_value(i + 1);
        }

      tensor.set_allocated_shape(new onnx::TensorShapeProto(std::move(shapeProto)));
      tensor.set_elem_type(onnx::TensorProto_DataType_INT64);

      typeProto.set_allocated_tensor_type(new onnx::TypeProto_Tensor(std::move(tensor)));

      value.set_allocated_type(new onnx::TypeProto(std::move(typeProto)));
      value.set_name("Test");

      Dense_tensor d(value); // total memory 2*2*64=256 bits

      lhs = d.compute_memory_usage();
    }

    memory_type rhs = 24 * sizeof(int64_t);

    ASSERT_EQ(lhs, rhs);
  }
}