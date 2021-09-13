//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>

#include "../src/Butcher.h"

TEST(ButcherTest, compute_two_slice_brute_force_test) {
  using Type_info_pointer = std::shared_ptr<Type_info>;
  using Layers = Node<Type_info>;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  Butcher butcher(model_path);


}

TEST(ButcherTest, compute_partial_two_slice_memory_brute_force_test) {
  using Type_info_pointer = std::shared_ptr<Type_info>;
  using Layers = Node<Type_info>;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Layers> graph(model_path, true);

  size_t half_size = graph.compute_memory_usage_input() / 2;

  Butcher butcher(std::move(graph));

  auto tot = butcher.compute_partial_two_slice_memory_brute_force(half_size);

  std::cout << std::endl;
}