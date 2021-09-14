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