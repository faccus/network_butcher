//
// Created by faccus on 11/09/21.
//

#include <gtest/gtest.h>
#include "../../src/Network/Graph.h"
#include "../TestClass.h"

TEST(GraphTests, ConstructorFromModel) {
  using Input = graph_input_type;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  onnx::ModelProto model_test;
  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  utilities::parse_onnx_file(model_test, model_path);

  Graph<Input> graph(model_test, true);
  Graph<Input> graph2(model_test);

  std::cout << std::endl;
}

TEST(GraphTests, ConstructorFromString) {
  using Input = graph_input_type;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Input> graph(model_path, true);
  Graph<Input> graph2(model_path);

  std::cout << std::endl;
}

TEST(GraphTests, ConstructorFromGraph)
{
  using Input = graph_input_type;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Input> graph(model_path, true);
  Graph<Input> graph2(std::move(graph));
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
  int number_of_nodes = 10;

  std::vector<node_type> nodes;
  nodes.emplace_back(0,
                     io_id_collection_type(),
                     io_id_collection_type{0},
                     io_id_collection_type());

  for (int i = 1; i < number_of_nodes - 1; ++i)
    nodes.emplace_back(i,
                       io_id_collection_type{(i-1)*10},
                       io_id_collection_type{i*10},
                       io_id_collection_type{});

  nodes.emplace_back(number_of_nodes-1,
                     io_id_collection_type{(number_of_nodes-2) * 10},
                     io_id_collection_type{},
                     io_id_collection_type{});

  std::map<io_id_type, Input> map;
  for(io_id_type i = 0; i < 2 * number_of_nodes; ++i)
    map[i*10] = Input(i+1);

  Graph<Input> graph_cons(nodes, map);

  std::cout << std::endl;
}