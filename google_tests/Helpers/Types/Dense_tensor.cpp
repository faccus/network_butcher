//
// Created by faccus on 30/08/21.
//

#include <gtest/gtest.h>
#include "../../../src/Helpers/Types/Dense_tensor.h"

TEST(DenseTensorTest, ConstuctorAndMemoryUsage) {
  Dense_tensor d(onnx::TensorProto_DataType_INT64,
                 {1, 1, 2, 2}); // total memory 2*2*64=256 bits

  auto res = d.compute_memory_usage();

  ASSERT_TRUE(res == 4 * sizeof(int64_t));
}

TEST(DenseTensorTest, ConstuctorValueInfoProto) {
  onnx::ValueInfoProto value;
  onnx::TypeProto typeProto;
  onnx::TypeProto_Tensor tensor;
  onnx::TensorShapeProto shapeProto;

    for(int i = 0; i < 4; ++i)
    {
      auto dim = shapeProto.add_dim();
      dim->set_dim_value(i+1);
    }

  tensor.set_allocated_shape(&shapeProto);
  tensor.set_elem_type(onnx::TensorProto_DataType_INT64);

  typeProto.set_allocated_tensor_type(&tensor);

  value.set_allocated_type(&typeProto);
  value.set_name("Test");

  Dense_tensor d(value); // total memory 2*2*64=256 bits

  auto res = d.compute_memory_usage();

  ASSERT_TRUE(res == 24 * sizeof(int64_t));
}