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
}

TEST(GraphTests, ConstructorFromString) {
  using Input = graph_input_type;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Input> graph(model_path, true);
  Graph<Input> graph2(model_path);
}

TEST(GraphTests, ConstructorFromGraph) {
  using Input = graph_input_type;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Input> graph(model_path, true);
  Graph<Input> graph2(std::move(graph));
}

TEST(GraphTests, ConstructorFromCustomClass) {
  using Input = TestMemoryUsage;

  Graph<Input> graph_empty;

  std::vector<node_type> nodes;
  nodes.emplace_back(0,
                     io_id_collection_type(),
                     io_id_collection_type{0},
                     io_id_collection_type());

  for (int i = 1; i < 9; ++i)
    nodes.emplace_back(i,
                       io_id_collection_type{i - 1},
                       io_id_collection_type{i + 1},
                       io_id_collection_type{});

  nodes.emplace_back(9,
                     io_id_collection_type{8},
                     io_id_collection_type{},
                     io_id_collection_type{});

  std::map<io_id_type, Input> map;
  for(io_id_type i = 0; i < 9; ++i)
    map[i] = Input(i+1);

  Graph<Input> graph_cons(nodes, map);

  std::cout << std::endl;
}