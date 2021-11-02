//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

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
  const Computer_memory computer{};

  size_t half_size = computer.compute_memory_usage_input(graph) / 2;

  Butcher<Input> butcher(std::move(graph));

  auto tot = butcher.compute_two_slice_memory_brute_force(half_size);

  std::cout << std::endl;
}

TEST(ButcherTest, compute_k_shortest_paths_linear)
{
  using Input = graph_input_type;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Input>          graph(model_path, true);
  const Computer_memory computer{};
  Butcher<Input>        butcher(std::move(graph));

  std::map<edge_type, type_weight> weights;

  std::default_random_engine             generator;
  std::uniform_real_distribution<double> distribution(.0, 10.);

  std::function<type_weight(edge_type const &)> weights_fun =
    [&computer, &butcher, &weights, &generator, &distribution](
      edge_type const &edge) {
      auto const &graph = butcher.getGraph();

      if (edge.first < graph.nodes.size() && edge.first >= 0 &&
          edge.second < graph.nodes.size() && edge.second >= 0 &&
          edge.first != edge.second)
        {
          auto const it = weights.find(edge);
          if (it != weights.cend())
            return it->second;
          else
            {
              auto const weight = distribution(generator);
              weights.insert(std::make_pair(edge, weight));
              return weight;
            }
        }
      else
        return -1.;
    };

  auto const res = butcher.compute_k_shortest_paths_linear(weights_fun, 1, 5);

  std::cout << std::endl;
}