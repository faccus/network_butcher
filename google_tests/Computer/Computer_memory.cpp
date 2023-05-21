#include <gtest/gtest.h>
#include <memory>

#include "IO_Manager.h"
#include "TestClass.h"

#include "Computer_memory.h"


namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;
  using namespace network_butcher::computer;

  using basic_type    = int;
  using Input         = TestMemoryUsage<basic_type>;
  using Content_type  = Content<Input>;
  using Node_type     = CNode<Content_type>;
  using IO_collection = io_collection_type<Input>;
  using Graph_type    = WGraph<false, Node_type>;

  Graph_type
  basic_graph();

  TEST(ComputerTests, MemoryConctructorTest)
  {
    Computer_memory computer{};
  }

  TEST(ComputerTests, ComputeMemoryUsageTypeInfo)
  {
    Dense_tensor d(onnx::TensorProto_DataType_INT64, {1, 1, 2, 2}); // total memory 2*2*64=256 bits
    auto         res = Computer_memory::compute_memory_usage(d);
    ASSERT_EQ(res, 4 * sizeof(int64_t));

    {
      std::shared_ptr<Type_info> pointer = std::make_shared<Dense_tensor>(d);

      res = Computer_memory::compute_memory_usage(pointer);
      ASSERT_EQ(res, 4 * sizeof(int64_t));
    }

    {
      std::unique_ptr<Type_info> pointer = std::make_unique<Dense_tensor>(d);

      res = Computer_memory::compute_memory_usage(pointer);
      ASSERT_EQ(res, 4 * sizeof(int64_t));
    }
  }

  TEST(ComputerTests, ComputeMemoryUsageCustomClass)
  {
    using basic_type = int;

    TestMemoryUsage<basic_type> ex(10);

    ASSERT_EQ(Computer_memory::compute_memory_usage(ex), 10 * sizeof(basic_type));
  }

  TEST(ComputerTests, ComputeMemoryUsageGraphCustomClass)
  {
    auto graph_cons = basic_graph();

    auto lhs = Computer_memory::compute_memory_usage_input(graph_cons);
    auto rhs = sizeof(basic_type) * 10 * ((10 - 2) * (10 - 1) / 2);

    ASSERT_EQ(lhs, rhs);
  }

  Graph_type
  basic_graph()
  {
    std::vector<Node_type> nodes;
    Content_type           content(IO_collection(), IO_collection{{"X0", 0}}, IO_collection());

    nodes.emplace_back(std::move(content));

    for (int i = 1; i < 10 - 1; ++i)
      {
        content = Content_type(IO_collection{{"X" + std::to_string(i - 1), (i - 1) * 10}},
                               IO_collection{{"X" + std::to_string(i), i * 10}},
                               IO_collection{});

        nodes.emplace_back(std::move(content));
      }

    content =
      Content_type(IO_collection{{"X" + std::to_string(10 - 1), (10 - 2) * 10}}, IO_collection{}, IO_collection{});

    nodes.emplace_back(std::move(content));

    return Graph_type(nodes);
  }
} // namespace