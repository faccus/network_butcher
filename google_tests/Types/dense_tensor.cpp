#include <network_butcher/Types/dense_tensor.h>
#include <gtest/gtest.h>

// Check around dense_tensor

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;

  // Verify that it was successfully constructed (and check of memory usage)
  TEST(DenseTensorTest, ConstuctorAndMemoryUsage)
  {
    Dense_tensor d(onnx::TensorProto_DataType_INT64, {1, 1, 2, 2}); // total memory 2*2*64=256 bits

    auto res = d.compute_memory_usage();

    ASSERT_EQ(res, 4 * sizeof(int64_t));
  }

  // Verify that the memory usage computation is OK and that the object is built without any errors.
  TEST(DenseTensorTest, ConstuctorValueInfoProto)
  {
    Memory_Type lhs = 0;

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

    Memory_Type rhs = 24 * sizeof(int64_t);

    ASSERT_EQ(lhs, rhs);
  }
} // namespace