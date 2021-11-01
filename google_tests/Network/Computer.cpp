//
// Created by faccus on 15/10/21.
//

#include <gtest/gtest.h>
#include <memory>

#include "../../src/Helpers/Computer/Computer_memory.h"
#include "../TestClass.h"

TEST(ComputerTests, ConctructorTest)
{
  Computer_memory computer{};
}

TEST(ComputerTests, ComputeMemoryUsageTypeInfo)
{
  Computer_memory computer;
  Dense_tensor    d(onnx::TensorProto_DataType_INT64,
                    {1, 1, 2, 2}); // total memory 2*2*64=256 bits
  auto            res = computer.compute(d);
  ASSERT_EQ(res, 4 * sizeof(int64_t));

  {
    std::shared_ptr<Type_info> pointer = std::make_shared<Dense_tensor>(d);

    res = computer.compute(pointer);
    ASSERT_EQ(res, 4 * sizeof(int64_t));
  }

  {
    std::unique_ptr<Type_info> pointer = std::make_unique<Dense_tensor>(d);

    res = computer.compute(pointer);
    ASSERT_EQ(res, 4 * sizeof(int64_t));
  }
}

TEST(ComputerTests, ComputeMemoryUsageCustomClass)
{
  using basic_type = int;

  Computer_memory             computer;
  TestMemoryUsage<basic_type> ex(10);

  ASSERT_EQ(computer.compute(ex), 10 * sizeof(basic_type));
}

TEST(ComputerTests, ComputeMemoryUsageGraphCustomClass)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  Computer_memory computer;
  int             number_of_nodes = 10;

  std::vector<node_type> nodes;
  nodes.emplace_back(0,
                     io_id_collection_type(),
                     io_id_collection_type{0},
                     io_id_collection_type());

  for (int i = 1; i < number_of_nodes - 1; ++i)
    nodes.emplace_back(i,
                       io_id_collection_type{(i - 1) * 10},
                       io_id_collection_type{i * 10},
                       io_id_collection_type{});

  nodes.emplace_back(number_of_nodes - 1,
                     io_id_collection_type{(number_of_nodes - 2) * 10},
                     io_id_collection_type{},
                     io_id_collection_type{});

  std::map<io_id_type, Input> map;
  for (io_id_type i = 0; i < 2 * number_of_nodes; ++i)
    map[i * 10] = Input(i + 1);

  Graph<Input> graph_cons(nodes, map);
  auto         lhs = computer.compute_memory_usage_input(graph_cons);
  auto rhs = sizeof(basic_type) * (number_of_nodes * (number_of_nodes - 1) / 2);

  ASSERT_EQ(lhs, rhs);
}
