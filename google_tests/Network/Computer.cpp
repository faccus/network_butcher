//
// Created by faccus on 15/10/21.
//

#include <gtest/gtest.h>
#include <memory>

#include "../../src/Network/Computer.h"

TEST(ComputerTests, ConctructorTest) {
  Computer computer{};
}

TEST(ComputerTests, ComputeMemoryUsageTypeInfo) {
  Computer computer;
  Dense_tensor d(onnx::TensorProto_DataType_INT64,
                 {1, 1, 2, 2}); // total memory 2*2*64=256 bits
  auto res = computer.compute_memory_usage(d);
  ASSERT_TRUE(res == 4 * sizeof(int64_t));

  {
    std::shared_ptr<Type_info> pointer = std::make_shared<Dense_tensor>(d);

    res = computer.compute_memory_usage(pointer);
    ASSERT_TRUE(res == 4 * sizeof(int64_t));
  }

  {
    std::unique_ptr<Type_info> pointer = std::make_unique<Dense_tensor>(d);

    res = computer.compute_memory_usage(pointer);
    ASSERT_TRUE(res == 4 * sizeof(int64_t));
  }
}
