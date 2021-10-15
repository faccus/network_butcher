//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>

#include "../src/Butcher.h"

TEST(ButcherTest, compute_two_slice_brute_force_test) {
  using Input = graph_input_type;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  Butcher<Input> butcher(model_path);
  auto res = butcher.compute_two_slice_brute_force();
}

TEST(ButcherTest, compute_two_slice_memory_brute_force_test)
{
  using Input = graph_input_type;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Input> graph(model_path, true);
  const Computer computer{};

  size_t half_size = computer.compute_memory_usage_input(graph) / 2;

  Butcher<Input> butcher(std::move(graph));

  auto tot = butcher.compute_two_slice_memory_brute_force(half_size);

  std::cout << std::endl;
}