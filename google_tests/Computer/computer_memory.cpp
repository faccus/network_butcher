#include <gtest/gtest.h>
#include <memory>

#include "../test_class.h"

#include <network_butcher/Computer/computer_memory.h>

/// Check if the memory usage functions work

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;
  using namespace network_butcher::computer;

  using basic_type    = int;
  using Input         = Test_Class<basic_type>;
  using Content_type  = Content<Input>;
  using Node_type     = CNode<Content_type>;
  using Graph_type    = WGraph<false, Node_type>;

  auto
  basic_graph() -> Graph_type;

  /// Check memory usage of simple Dense Tensor
  TEST(ComputerTests, ComputeMemoryUsageDenseTensor)
  {
    Dense_tensor d(onnx::TensorProto_DataType_INT64, {1, 1, 2, 2}); // total memory 2*2*64=256 bits
    auto         res = Computer_memory::compute_memory_usage(d);
    ASSERT_EQ(res, 4 * sizeof(int64_t));

    {
      std::shared_ptr<Type_Info> pointer = std::make_shared<Dense_tensor>(d);

      res = Computer_memory::compute_memory_usage(pointer);
      ASSERT_EQ(res, 4 * sizeof(int64_t));
    }

    {
      std::unique_ptr<Type_Info> pointer = std::make_unique<Dense_tensor>(d);

      res = Computer_memory::compute_memory_usage(pointer);
      ASSERT_EQ(res, 4 * sizeof(int64_t));
    }
  }

  /// Check memory usage of sample class
  TEST(ComputerTests, ComputeMemoryUsageCustomClass)
  {
    Test_Class<basic_type> ex(10);

    ASSERT_EQ(Computer_memory::compute_memory_usage(ex), 10 * sizeof(basic_type));
  }

  /// Check memory usage of sample graph
  TEST(ComputerTests, ComputeMemoryUsageGraphCustomClass)
  {
    auto graph_cons = basic_graph();

    auto lhs = Computer_memory::compute_memory_usage_input(graph_cons);
    auto rhs = sizeof(basic_type) * 10 * ((10 - 2) * (10 - 1) / 2);

    ASSERT_EQ(lhs, rhs);
  }


  auto
  basic_graph() -> Graph_type
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(std::move(Content_Builder<Input>().set_output({{"X0", 0}})).build());

    for (int i = 1; i < 10 - 1; ++i)
      {
        nodes.emplace_back(std::move(Content_Builder<Input>()
                                       .set_input({{"X" + std::to_string(i - 1), (i - 1) * 10}})
                                       .set_output({{"X" + std::to_string(i), i * 10}}))
                             .build());
      }

    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X" + std::to_string(10 - 1), (10 - 2) * 10}})).build());

    return Graph_type(nodes);
  }
} // namespace