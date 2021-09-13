//
// Created by faccus on 11/09/21.
//

#include <gtest/gtest.h>
#include "../../src/Network/Graph.h"

TEST(GraphTests, ConstructorFromModel) {
  using Layers = Node<Type_info>;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  onnx::ModelProto model_test;
  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  utilities::parse_onnx_file(model_test, model_path);

  Graph<Layers> graph(model_test, true);
  Graph<Layers> graph2(model_test);
}

TEST(GraphTests, ConstructorFromString) {
  using Layers = Node<Type_info>;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Layers> graph(model_path, true);
  Graph<Layers> graph2(model_path);
}

TEST(GraphTests, MemoryUsage) {
  using Layers = Node<Type_info>;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  Graph<Layers> graph(model_path, true);

  auto memory_usage       = graph.compute_memory_usage();
  auto nodes_memory_usage = graph.compute_nodes_memory_usage();
  size_t tmp = 0;
  for(auto & n : nodes_memory_usage)
    tmp += n;

  ASSERT_TRUE(tmp == memory_usage);
}

TEST(GraphTests, MemoryUsageInput) {
  using Layers = Node<Type_info>;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  Graph<Layers> graph(model_path, true);

  auto memory_usage       = graph.compute_memory_usage_input();
  auto nodes_memory_usage = graph.compute_nodes_memory_usage_input();
  size_t tmp = 0;
  for(auto & n : nodes_memory_usage)
    tmp += n;

  ASSERT_TRUE(tmp == memory_usage);
}

TEST(GraphTests, MemoryUsageOutput) {
  using Layers = Node<Type_info>;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  Graph<Layers> graph(model_path, true);

  auto memory_usage       = graph.compute_memory_usage_output();
  auto nodes_memory_usage = graph.compute_nodes_memory_usage_output();
  size_t tmp = 0;
  for(auto & n : nodes_memory_usage)
    tmp += n;

  ASSERT_TRUE(tmp == memory_usage);
}