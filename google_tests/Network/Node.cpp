//
// Created by faccus on 13/09/21.
//

#include <gtest/gtest.h>
#include <memory>

#include "../../src/Network/Node.h"
#include "../../src/Helpers/Types/Dense_tensor.h"

TEST(NodeTests, ConctructorTest) {
  std::vector<long> shape_input{1, 1, 2, 2};
  std::vector<long> shape_output{3, 1, 1 ,1};

  std::vector<std::shared_ptr<Dense_tensor>> input;
  input.push_back(
    std::make_shared<Dense_tensor>(onnx::TensorProto_DataType_INT64,
                                   shape_input));
  std::vector<std::shared_ptr<Dense_tensor>> output;
  output.push_back(
    std::make_shared<Dense_tensor>(onnx::TensorProto_DataType_BOOL,
                                   shape_output));

  Node<Dense_tensor> node(1, input, output);

  auto mem_out = node.compute_memory_usage_output();
  auto mem_in  = node.compute_memory_usage_input();
  auto mem_tot = node.compute_memory_usage();

  ASSERT_TRUE(mem_out == 3 * sizeof(bool));
  ASSERT_TRUE(mem_in == 4 * sizeof(int64_t));
  ASSERT_TRUE(mem_tot == (4 * sizeof(int64_t) + 3 * sizeof(bool)));
}