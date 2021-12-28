//
// Created by faccus on 15/10/21.
//

#include <gtest/gtest.h>
#include <memory>

#include "../../../src/Helpers/Computer/Computer_memory.h"
#include "../../../src/Helpers/Computer/Computer_time.h"
#include "../../TestClass.h"

namespace ComputerMemoryTests
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  Graph<Input>
  basic_graph(int);

  TEST(ComputerTests, MemoryConctructorTest)
  {
    Computer_memory computer{};
  }

  TEST(ComputerTests, ComputeMemoryUsageTypeInfo)
  {
    Computer_memory computer;
    Dense_tensor    d(onnx::TensorProto_DataType_INT64,
                   {1, 1, 2, 2}); // total memory 2*2*64=256 bits
    auto            res = computer.compute_memory_usage(d);
    ASSERT_EQ(res, 4 * sizeof(int64_t));

    {
      std::shared_ptr<Type_info> pointer = std::make_shared<Dense_tensor>(d);

      res = computer.compute_memory_usage(pointer);
      ASSERT_EQ(res, 4 * sizeof(int64_t));
    }

    {
      std::unique_ptr<Type_info> pointer = std::make_unique<Dense_tensor>(d);

      res = computer.compute_memory_usage(pointer);
      ASSERT_EQ(res, 4 * sizeof(int64_t));
    }
  }

  TEST(ComputerTests, ComputeMemoryUsageCustomClass)
  {
    using basic_type = int;

    Computer_memory             computer;
    TestMemoryUsage<basic_type> ex(10);

    ASSERT_EQ(computer.compute_memory_usage(ex), 10 * sizeof(basic_type));
  }

  TEST(ComputerTests, ComputeMemoryUsageGraphCustomClass)
  {
    Computer_memory computer;
    int             number_of_nodes = 10;

    auto graph_cons = basic_graph(number_of_nodes);

    auto lhs = computer.compute_memory_usage_input(graph_cons);
    auto rhs =
      sizeof(basic_type) * (number_of_nodes * (number_of_nodes - 1) / 2);

    ASSERT_EQ(lhs, rhs);
  }

  Graph<Input>
  basic_graph(int number_of_nodes)
  {
    std::vector<node_type> nodes;
    nodes.emplace_back(io_id_collection_type(),
                       io_id_collection_type{{"Y", 0}},
                       io_id_collection_type());

    for (int i = 1; i < number_of_nodes - 1; ++i)
      nodes.emplace_back(io_id_collection_type{{"X", (i - 1) * 10}},
                         io_id_collection_type{{"Y", i * 10}},
                         io_id_collection_type{});

    nodes.emplace_back(io_id_collection_type{{"X", (number_of_nodes - 2) * 10}},
                       io_id_collection_type{},
                       io_id_collection_type{});

    std::map<io_id_type, Input> map;
    for (io_id_type i = 0; i < 2 * number_of_nodes; ++i)
      map[i * 10] = Input(i + 1);

    return Graph<Input>(nodes, map);
  }
}

namespace ComputerTimeTests
{
  Graph<graph_input_type>
  basic_graph();

  TEST(ComputerTests, TimeConstructorTest)
  {
    Computer_time computer;
  }

  TEST(ComputerTests, CheckFactory)
  {
    Computer_time           computer;
    Hardware_specifications test_hw("test_device");

    test_hw.set_regression_coefficient("batchnormalization", {1., .5});
    auto       graph = basic_graph();
    auto const time =
      computer.compute_operation_time(graph, graph.nodes.front(), test_hw);

    ASSERT_EQ(time, 9);
  }

  Graph<graph_input_type>
  basic_graph()
  {
    return Graph<graph_input_type>("resnet18-v2-7-inferred.onnx");
  }
} // namespace ComputerTimeTests