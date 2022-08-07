//
// Created by faccus on 11/09/21.
//

#include "../../include/Network/Graph.h"
#include "../../include/IO_Manager.h"
#include "../TestClass.h"
#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher_types;

  TEST(GraphTests, Constructor)
  {
    using Input = graph_input_type;
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    onnx::ModelProto  model_test;
    const std::string model_path = "version-RFB-640-inferred.onnx";
    network_butcher_utilities::parse_onnx_file(model_test, model_path);

    MWGraph graph = std::get<0>(network_butcher_io::IO_Manager::import_from_onnx(model_path));
  }

  TEST(GraphTests, ConstructorFromGraph)
  {
    using Input = graph_input_type;
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    const std::string model_path = "resnet18-v2-7-inferred.onnx";

    MWGraph graph = std::get<0>(network_butcher_io::IO_Manager::import_from_onnx(model_path));
    MWGraph graph2(std::move(graph));
  }

  TEST(GraphTests, DefaultConstructors)
  {
    using basic_type = int;
    using Input      = TestMemoryUsage<basic_type>;

    Graph<Input>            graph_empty;
    Graph<graph_input_type> graph_empty2;
  }

  TEST(GraphTests, ConstructorFromCustomClass)
  {
    using basic_type    = int;
    using Input         = TestMemoryUsage<basic_type>;
    using Content_type  = Content<Input>;
    using Graph_type    = Graph<Content_type>;
    using IO_collection = io_collection_type<Input>;
    using Node_type     = Graph_type::Node_Type;

    int number_of_nodes = 10;

    std::vector<Node_type> nodes;

    auto content = Content_type::make_content(IO_collection{}, IO_collection{{"X", 0}});

    nodes.emplace_back(std::move(content));

    for (int i = 1; i < number_of_nodes - 1; ++i)
      {
        content = Content_type(IO_collection{{"X", (i - 1) * 10}}, IO_collection{{"X", i * 10}}, IO_collection{});

        nodes.emplace_back(std::move(content));
      }

    content = Content_type(IO_collection{{"X", (number_of_nodes - 2) * 10}}, IO_collection{}, IO_collection{});

    nodes.emplace_back(std::move(content));

    Graph_type graph(nodes);
  }

  TEST(GraphTests, RemoveNodes)
  {
    using basic_type    = int;
    using Input         = TestMemoryUsage<basic_type>;
    using Content_type  = Content<Input>;
    using Graph_type    = Graph<Content_type>;
    using IO_collection = io_collection_type<Input>;
    using Node_type     = Graph_type::Node_Type;

    int number_of_nodes = 10;

    std::vector<Node_type> nodes;
    Content_type           content(IO_collection(), IO_collection{{"X0", 3}}, IO_collection());

    nodes.emplace_back(std::move(content));

    for (int i = 1; i < number_of_nodes - 1; ++i)
      {
        content = Content_type(IO_collection{{"X" + std::to_string((i - 1) * 10), 0}},
                               IO_collection{{"X" + std::to_string(i * 10), 1}},
                               IO_collection{});

        nodes.emplace_back(std::move(content));
      }

    content = Content_type(IO_collection{{"X" + std::to_string((number_of_nodes - 2) * 10), 2}},
                           IO_collection{},
                           IO_collection{});

    nodes.emplace_back(std::move(content));

    Graph<Content<Input>> graph(nodes);

    graph.remove_nodes({3, 5, 6, 1, 11});

    EXPECT_TRUE(graph.size() == 6);
    EXPECT_TRUE(graph[5].content.get_input().begin()->first == "X80");
  }

} // namespace