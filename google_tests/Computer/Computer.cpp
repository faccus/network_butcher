//
// Created by faccus on 15/10/21.
//

#include <gtest/gtest.h>
#include <memory>

#include "Computer_flops.h"
#include "Computer_memory.h"
#include "TestClass.h"

#include "IO_Manager.h"

namespace ComputerMemoryTests
{
  using namespace network_butcher_types;
  using namespace network_butcher_computer;

  using basic_type    = int;
  using Input         = TestMemoryUsage<basic_type>;
  using Content_type  = Content<Input>;
  using IO_collection = io_collection_type<Input>;
  using Graph_type    = MWGraph<Content_type>;
  using Node_type     = Graph_type::Node_Type;

  Graph_type
  basic_graph(int);

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
    int number_of_nodes = 10;

    auto graph_cons = basic_graph(number_of_nodes);

    auto lhs = Computer_memory::compute_memory_usage_input(graph_cons);
    auto rhs = sizeof(basic_type) * 10 * ((number_of_nodes - 2) * (number_of_nodes - 1) / 2);

    ASSERT_EQ(lhs, rhs);
  }

  Graph_type
  basic_graph(int number_of_nodes)
  {
    std::vector<Node_type> nodes;
    Content_type           content(IO_collection(), IO_collection{{"X0", 0}}, IO_collection());

    nodes.emplace_back(std::move(content));

    for (int i = 1; i < number_of_nodes - 1; ++i)
      {
        content = Content_type(IO_collection{{"X" + std::to_string(i - 1), (i - 1) * 10}},
                               IO_collection{{"X" + std::to_string(i), i * 10}},
                               IO_collection{});

        nodes.emplace_back(std::move(content));
      }

    content = Content_type(IO_collection{{"X" + std::to_string(number_of_nodes - 1), (number_of_nodes - 2) * 10}},
                           IO_collection{},
                           IO_collection{});

    nodes.emplace_back(std::move(content));

    return WGraph<Content<Input>>(nodes);
  }
} // namespace ComputerMemoryTests

namespace ComputerFlopsTests
{
  using namespace network_butcher_types;
  using namespace network_butcher_computer;


  TEST(CompiterFlopsTests, ComputeFlopsGraph)
  {
    std::string const path = "test_data/models/resnet18-v2-7-inferred.onnx";
    auto graph = std::get<0>(
      network_butcher_io::IO_Manager::import_from_onnx(path, false, true, 3, true));

    for (auto const &node : graph.get_nodes())
      {
        auto const idk = Computer_flops::compute_macs_flops(node.content);
        std::cout << std::endl;
      }

    std::cout << std::endl;
  }

} // namespace ComputerMemoryTests