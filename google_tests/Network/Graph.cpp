//
// Created by faccus on 11/09/21.
//

#include <gtest/gtest.h>
#include "../../src/Network/Graph.h"
#include "../../src/Helpers/IO_Manager.h"
#include "../TestClass.h"

TEST(GraphTests, Constructor) {
  using Input = graph_input_type;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  onnx::ModelProto model_test;
  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  utilities::parse_onnx_file(model_test, model_path);


  WGraph graph = IO_Manager::import_from_onnx(model_path).first;

  std::cout << std::endl;
}

TEST(GraphTests, ConstructorFromGraph)
{
  using Input = graph_input_type;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  WGraph        graph = IO_Manager::import_from_onnx(model_path).first;
  WGraph graph2(std::move(graph));
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
  using IO_collection = io_collection_type<Input>;
  using Node_type     = Node<Content<Input>>;

  int number_of_nodes = 10;

  std::vector<Node_type> nodes;
  Content content(IO_collection(), IO_collection{{"Y", 0}}, IO_collection());

  nodes.emplace_back(std::move(content));

  for (int i = 1; i < number_of_nodes - 1; ++i)
    {
      content = Content(IO_collection{{"X", (i - 1) * 10}},
                        IO_collection{{"Y", i * 10}},
                        IO_collection{});

      nodes.emplace_back(std::move(content));
    }

  content = Content(IO_collection{{"X", (number_of_nodes - 2) * 10}},
                    IO_collection{},
                    IO_collection{});

  nodes.emplace_back(std::move(content));

  Graph<Content<Input>> graph(nodes);

  std::cout << std::endl;
}
