//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/Butcher.h"
#include "TestClass.h"

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
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::size_t num_devices = 3;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.push_back(node_type(0, {}, {0}));
  nodes.push_back(node_type(1, {0}, {1}));
  nodes.push_back(node_type(2, {1}, {2}));
  nodes.push_back(node_type(3, {1}, {3}));
  nodes.push_back(node_type(4, {3}, {4}));
  nodes.push_back(node_type(5, {2, 4}, {5}));
  nodes.push_back(node_type(6, {5}, {6}));

  for (io_id_type i = 0; i < nodes.size(); ++i)
    map[i] = i;

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));

  type_collection_weights weight_map;
  weight_map[{0, 1}] = 1000.;
  weight_map[{1, 2}] = 1000.;
  weight_map[{2, 5}] = 1000.;
  weight_map[{5, 6}] = 1000.;
  weight_map[{4, 5}] = 1000.;
  weight_map[{1, 3}] = 500.;
  weight_map[{3, 4}] = 500.;

  type_collection_weights additional_weights;

  for (auto const &edge : weight_map)
    {
      for (std::size_t i = 1; i < num_devices; ++i)
        {
          additional_weights[{edge.first.first + nodes.size() * i,
                              edge.first.second + nodes.size() * i}] =
            weight_map[edge.first] / std::pow(2, i);
        }
    }
  weight_map.merge(std::move(additional_weights));

  std::function<type_weight(edge_type const &)> weight_fun =
    [&](edge_type const &edge) {
      auto const &graph = butcher.getGraph();

      if (edge.first >= 0 && edge.first < num_devices * graph.nodes.size() &&
          edge.second >= 0 && edge.second < num_devices * graph.nodes.size())
        {
          auto const it = weight_map.find(edge);
          if (it != weight_map.cend())
            return it->second;
          else
            return -1.;
        }
      else
        return -1.;
    };

  std::function<type_weight(edge_type const &)> transmission_fun =
    [&](edge_type const &edge) {
      auto const &graph = butcher.getGraph();
      if (edge.first >= 0 && edge.first < num_devices * graph.nodes.size() &&
          edge.second >= 0 && edge.second < num_devices * graph.nodes.size())
        {
          auto in_device_id  = edge.first / graph.nodes.size();
          auto out_device_id = edge.second / graph.nodes.size();

          if (in_device_id > out_device_id)
            std::swap(in_device_id, out_device_id);

          if (out_device_id - in_device_id == 2)
            return 1000.;
          else if (out_device_id - in_device_id == 1)
            {
              if (out_device_id == 2)
                return 700.;
              else
                return 300.;
            }
          else
            return .0;
        }
      else
        return -1.;
    };


  auto const res = butcher.compute_k_shortest_paths_linear(weight_fun,
                                                           transmission_fun,
                                                           num_devices,
                                                           1000);

  std::cout << std::endl;
}