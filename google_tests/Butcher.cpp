//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/Butcher.h"
#include "../src/Helpers/APCS/chrono.h"
#include "TestClass.h"

TEST(ButcherTest, compute_two_slice_brute_force_test)
{
  using Input = graph_input_type;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  Butcher<Input>    butcher(model_path);
  auto              res = butcher.compute_two_slice_brute_force();
}

TEST(ButcherTest, compute_two_slice_memory_brute_force_test)
{
  using Input = graph_input_type;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Input>          graph(model_path, true);
  const Computer_memory computer{};

  size_t half_size = computer.compute_memory_usage_input(graph) / 2;

  Butcher<Input> butcher(std::move(graph));

  auto tot = butcher.compute_two_slice_memory_brute_force(half_size);

  std::cout << std::endl;
}


TEST(ButcherTest, compute_k_shortest_paths_eppstein_linear)
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
  nodes.push_back(node_type(7, {6}, {7}));

  for (io_id_type i = 0; i < nodes.size(); ++i)
    map[i] = i;

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));

  type_collection_weights weight_map;

  weight_map[{0, 1}]  = 1000.;
  weight_map[{0, 8}]  = 500.;
  weight_map[{0, 13}] = 250.;

  weight_map[{1, 2}] = 1000.;
  weight_map[{1, 3}] = 500.;
  weight_map[{3, 4}] = 500.;
  weight_map[{2, 5}] = 1000.;
  weight_map[{4, 5}] = 1000.;
  weight_map[{5, 6}] = 1000.;


  weight_map[{6, 7}]  = 0.;
  weight_map[{13, 7}] = 0.;
  weight_map[{19, 7}] = 0.;


  type_collection_weights additional_weights;
  auto const             &graph     = butcher.getGraph();
  auto const              num_nodes = graph.nodes.size();

  for (auto const &edge : weight_map)
    {
      if (edge.first.first == 0)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first,
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      else if (edge.first.second == num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      if (edge.first.first != 0 && edge.first.second != num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
    }
  weight_map.merge(std::move(additional_weights));

  std::function<type_weight(edge_type const &)> weight_fun =
    [&](edge_type const &edge) {
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
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
      auto const &graph     = butcher.getGraph();
      auto const  num_nodes = graph.nodes.size();
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
        {
          auto in_device_id =
            edge.first < num_nodes ? 0 : (edge.first - 2) / (num_nodes - 2);
          auto out_device_id =
            edge.second < num_nodes ? 0 : (edge.second - 2) / (num_nodes - 2);

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
            return 0.;
        }
      else
        return -1.;
    };


  auto const res = butcher.compute_k_shortest_paths_eppstein_linear(
    weight_fun, transmission_fun, num_devices, 1000);

  for (auto i = 0; i < res.size(); ++i)
    for (auto j = 0; j < res.size(); ++j)
      if (i != j && !(res[i] < res[j] || res[j] < res[i]))
        ASSERT_TRUE(!(res[i] < res[j] || res[j] < res[i]));

  ASSERT_EQ(res.size(), 81);
}

TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_linear)
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
  nodes.push_back(node_type(7, {6}, {7}));

  for (io_id_type i = 0; i < nodes.size(); ++i)
    map[i] = i;

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));

  type_collection_weights weight_map;

  weight_map[{0, 1}]  = 1000.;
  weight_map[{0, 8}]  = 500.;
  weight_map[{0, 13}] = 250.;

  weight_map[{1, 2}] = 1000.;
  weight_map[{1, 3}] = 500.;
  weight_map[{3, 4}] = 500.;
  weight_map[{2, 5}] = 1000.;
  weight_map[{4, 5}] = 1000.;
  weight_map[{5, 6}] = 1000.;


  weight_map[{6, 7}]  = 0.;
  weight_map[{13, 7}] = 0.;
  weight_map[{19, 7}] = 0.;


  type_collection_weights additional_weights;
  auto const             &graph     = butcher.getGraph();
  auto const              num_nodes = graph.nodes.size();

  for (auto const &edge : weight_map)
    {
      if (edge.first.first == 0)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first,
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      else if (edge.first.second == num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      if (edge.first.first != 0 && edge.first.second != num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
    }
  weight_map.merge(std::move(additional_weights));

  std::function<type_weight(edge_type const &)> weight_fun =
    [&](edge_type const &edge) {
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
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
      auto const &graph     = butcher.getGraph();
      auto const  num_nodes = graph.nodes.size();
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
        {
          auto in_device_id =
            edge.first < num_nodes ? 0 : (edge.first - 2) / (num_nodes - 2);
          auto out_device_id =
            edge.second < num_nodes ? 0 : (edge.second - 2) / (num_nodes - 2);

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
            return 0.;
        }
      else
        return -1.;
    };


  auto const res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
    weight_fun, transmission_fun, num_devices, 1000);

  for (auto i = 0; i < res.size(); ++i)
    for (auto j = 0; j < res.size(); ++j)
      if (i != j && !(res[i] < res[j] || res[j] < res[i]))
        ASSERT_TRUE(!(res[i] < res[j] || res[j] < res[i]));

  ASSERT_EQ(res.size(), 81);
}


TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_random)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;
  type_collection_weights     weight_map;

  std::size_t       num_devices = 3;
  std::size_t const num_nodes   = 10000;

  nodes.push_back(node_type(0, {}, {0}));
  for (int n = 1; n < num_nodes - 1; ++n)
    nodes.push_back(node_type(n, {n - 1}, {n}));
  nodes.push_back(node_type(num_nodes - 1, {num_nodes - 2}, {0}));

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));
  auto const    &graph = butcher.getGraph();

  std::random_device         rd;
  std::default_random_engine random_engine{rd()};
  type_collection_weights    additional_weights;

  std::uniform_real_distribution node_weights_generator{5000., 10000.};

  for (node_id_type i = 0; i < num_nodes - 1; ++i)
    weight_map[{i, i + 1}] = node_weights_generator(random_engine);
  for (std::size_t i = 1; i < num_devices; ++i)
    {
      weight_map[{0, num_nodes + (num_nodes - 2) * (i - 1)}] =
        node_weights_generator(random_engine) / std::pow(2, i);
      weight_map[{num_nodes + num_nodes - 3 + (num_nodes - 2) * (i - 1),
                  num_nodes - 1}] =
        node_weights_generator(random_engine) / std::pow(2, i);
    }

  for (auto const &edge : weight_map)
    {
      if (edge.first.first == 0)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first,
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      else if (edge.first.second == num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      if (edge.first.first != 0 && edge.first.second != num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
    }
  weight_map.merge(std::move(additional_weights));


  std::function<type_weight(edge_type const &)> weight_fun =
    [&](edge_type const &edge) {
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
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
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
        {
          auto in_device_id =
            edge.first < num_nodes ? 0 : (edge.first - 2) / (num_nodes - 2);
          auto out_device_id =
            edge.second < num_nodes ? 0 : (edge.second - 2) / (num_nodes - 2);

          if (in_device_id > out_device_id)
            std::swap(in_device_id, out_device_id);

          if (out_device_id - in_device_id == 2)
            return 10000.;
          else if (out_device_id - in_device_id == 1)
            {
              if (out_device_id == 2)
                return 7000.;
              else
                return 3000.;
            }
          else
            return 0.;
        }
      else
        return -1.;
    };

  Chrono crono;
  crono.start();
  auto const res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
    weight_fun, transmission_fun, num_devices, num_nodes * 0.1);
  crono.stop();

  crono.wallTime();


  std::cout << std::endl;
}

TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_multiple_random)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  std::size_t       num_devices     = 3;
  std::size_t const num_nodes       = 1000;
  std::size_t const number_of_tests = 100;

  nodes.push_back(node_type(0, {}, {0}));
  for (int n = 1; n < num_nodes - 1; ++n)
    nodes.push_back(node_type(n, {n - 1}, {n}));
  nodes.push_back(node_type(num_nodes - 1, {num_nodes - 2}, {0}));

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));
  auto const    &graph = butcher.getGraph();

  std::random_device             rd;
  std::default_random_engine     random_engine{rd()};
  std::uniform_real_distribution node_weights_generator{5000., 10000.};

  double total_time = .0;

  for (auto num_test = 0; num_test < number_of_tests; ++num_test)
    {
      type_collection_weights weight_map;
      type_collection_weights additional_weights;

      for (node_id_type i = 0; i < num_nodes - 1; ++i)
        weight_map[{i, i + 1}] = node_weights_generator(random_engine);
      for (std::size_t i = 1; i < num_devices; ++i)
        {
          weight_map[{0, num_nodes + (num_nodes - 2) * (i - 1)}] =
            node_weights_generator(random_engine) / std::pow(2, i);
          weight_map[{num_nodes + num_nodes - 3 + (num_nodes - 2) * (i - 1),
                      num_nodes - 1}] =
            node_weights_generator(random_engine) / std::pow(2, i);
        }

      for (auto const &edge : weight_map)
        {
          if (edge.first.first == 0)
            {
              for (std::size_t i = 1; i < num_devices; ++i)
                {
                  additional_weights[{edge.first.first,
                                      edge.first.second + nodes.size() - 1 +
                                        (i - 1) * (num_nodes - 2)}] =
                    weight_map[edge.first] / std::pow(2, i);
                }
            }
          else if (edge.first.second == num_nodes - 1)
            {
              for (std::size_t i = 1; i < num_devices; ++i)
                {
                  additional_weights[{edge.first.first + nodes.size() - 1 +
                                        (i - 1) * (num_nodes - 2),
                                      edge.first.second}] =
                    weight_map[edge.first] / std::pow(2, i);
                }
            }
          if (edge.first.first != 0 && edge.first.second != num_nodes - 1)
            {
              for (std::size_t i = 1; i < num_devices; ++i)
                {
                  additional_weights[{edge.first.first + nodes.size() - 1 +
                                        (i - 1) * (num_nodes - 2),
                                      edge.first.second + nodes.size() - 1 +
                                        (i - 1) * (num_nodes - 2)}] =
                    weight_map[edge.first] / std::pow(2, i);
                }
            }
        }
      weight_map.merge(std::move(additional_weights));


      std::function<type_weight(edge_type const &)> weight_fun =
        [&](edge_type const &edge) {
          if (edge.first >= 0 &&
              edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
              edge.second >= 0 &&
              edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
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
          if (edge.first >= 0 &&
              edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
              edge.second >= 0 &&
              edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
            {
              auto in_device_id =
                edge.first < num_nodes ? 0 : (edge.first - 2) / (num_nodes - 2);
              auto out_device_id = edge.second < num_nodes ?
                                     0 :
                                     (edge.second - 2) / (num_nodes - 2);

              if (in_device_id > out_device_id)
                std::swap(in_device_id, out_device_id);

              if (out_device_id - in_device_id == 2)
                return 10000.;
              else if (out_device_id - in_device_id == 1)
                {
                  if (out_device_id == 2)
                    return 7000.;
                  else
                    return 3000.;
                }
              else
                return 0.;
            }
          else
            return -1.;
        };

      Chrono crono;
      crono.start();
      auto const res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
        weight_fun, transmission_fun, num_devices, num_nodes * 0.1);
      crono.stop();

      total_time += crono.wallTime();
    }


  std::cout << "Average time per test: " << total_time / number_of_tests
            << "micro-seconds" << std::endl;
  ASSERT_GE(total_time / number_of_tests, 0);
}

TEST(ButcherTest, compute_k_shortest_paths_eppstein_random)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;
  type_collection_weights     weight_map;

  std::size_t       num_devices = 3;
  std::size_t const num_nodes   = 10000;

  nodes.push_back(node_type(0, {}, {0}));
  for (int n = 1; n < num_nodes - 1; ++n)
    nodes.push_back(node_type(n, {n - 1}, {n}));
  nodes.push_back(node_type(num_nodes - 1, {num_nodes - 2}, {0}));

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));
  auto const    &graph = butcher.getGraph();

  std::random_device         rd;
  std::default_random_engine random_engine{rd()};
  type_collection_weights    additional_weights;

  std::uniform_real_distribution node_weights_generator{5000., 10000.};

  for (node_id_type i = 0; i < num_nodes - 1; ++i)
    weight_map[{i, i + 1}] = node_weights_generator(random_engine);
  for (std::size_t i = 1; i < num_devices; ++i)
    {
      weight_map[{0, num_nodes + (num_nodes - 2) * (i - 1)}] =
        node_weights_generator(random_engine) / std::pow(2, i);
      weight_map[{num_nodes + num_nodes - 3 + (num_nodes - 2) * (i - 1),
                  num_nodes - 1}] =
        node_weights_generator(random_engine) / std::pow(2, i);
    }

  for (auto const &edge : weight_map)
    {
      if (edge.first.first == 0)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first,
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      else if (edge.first.second == num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      if (edge.first.first != 0 && edge.first.second != num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
    }
  weight_map.merge(std::move(additional_weights));


  std::function<type_weight(edge_type const &)> weight_fun =
    [&](edge_type const &edge) {
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
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
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
        {
          auto in_device_id =
            edge.first < num_nodes ? 0 : (edge.first - 2) / (num_nodes - 2);
          auto out_device_id =
            edge.second < num_nodes ? 0 : (edge.second - 2) / (num_nodes - 2);

          if (in_device_id > out_device_id)
            std::swap(in_device_id, out_device_id);

          if (out_device_id - in_device_id == 2)
            return 10000.;
          else if (out_device_id - in_device_id == 1)
            {
              if (out_device_id == 2)
                return 7000.;
              else
                return 3000.;
            }
          else
            return 0.;
        }
      else
        return -1.;
    };

  Chrono crono;
  crono.start();
  auto const res = butcher.compute_k_shortest_paths_eppstein_linear(
    weight_fun, transmission_fun, num_devices, num_nodes * 0.1);
  crono.stop();

  crono.wallTime();

  std::cout << std::endl;
}

TEST(ButcherTest, compute_k_shortest_paths_eppstein_vs_lazy_random)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;
  type_collection_weights     weight_map;

  std::size_t       num_devices = 3;
  std::size_t const num_nodes   = 10000;

  nodes.push_back(node_type(0, {}, {0}));
  for (int n = 1; n < num_nodes - 1; ++n)
    nodes.push_back(node_type(n, {n - 1}, {n}));
  nodes.push_back(node_type(num_nodes - 1, {num_nodes - 2}, {0}));

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));
  auto const    &graph = butcher.getGraph();

  std::random_device         rd;
  std::default_random_engine random_engine{rd()};
  type_collection_weights    additional_weights;

  std::uniform_real_distribution node_weights_generator{5000., 10000.};

  for (node_id_type i = 0; i < num_nodes - 1; ++i)
    weight_map[{i, i + 1}] = node_weights_generator(random_engine);
  for (std::size_t i = 1; i < num_devices; ++i)
    {
      weight_map[{0, num_nodes + (num_nodes - 2) * (i - 1)}] =
        node_weights_generator(random_engine) / std::pow(2, i);
      weight_map[{num_nodes + num_nodes - 3 + (num_nodes - 2) * (i - 1),
                  num_nodes - 1}] =
        node_weights_generator(random_engine) / std::pow(2, i);
    }

  for (auto const &edge : weight_map)
    {
      if (edge.first.first == 0)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first,
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      else if (edge.first.second == num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
      if (edge.first.first != 0 && edge.first.second != num_nodes - 1)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              additional_weights[{edge.first.first + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2),
                                  edge.first.second + nodes.size() - 1 +
                                    (i - 1) * (num_nodes - 2)}] =
                weight_map[edge.first] / std::pow(2, i);
            }
        }
    }
  weight_map.merge(std::move(additional_weights));


  std::function<type_weight(edge_type const &)> weight_fun =
    [&](edge_type const &edge) {
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
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
      if (edge.first >= 0 &&
          edge.first < num_nodes + (num_nodes - 2) * (num_devices - 1) &&
          edge.second >= 0 &&
          edge.second < num_nodes + (num_nodes - 2) * (num_devices - 1))
        {
          auto in_device_id =
            edge.first < num_nodes ? 0 : (edge.first - 2) / (num_nodes - 2);
          auto out_device_id =
            edge.second < num_nodes ? 0 : (edge.second - 2) / (num_nodes - 2);

          if (in_device_id > out_device_id)
            std::swap(in_device_id, out_device_id);

          if (out_device_id - in_device_id == 2)
            return 10000.;
          else if (out_device_id - in_device_id == 1)
            {
              if (out_device_id == 2)
                return 7000.;
              else
                return 3000.;
            }
          else
            return 0.;
        }
      else
        return -1.;
    };

  Chrono crono;
  crono.start();
  auto const res = butcher.compute_k_shortest_paths_eppstein_linear(
    weight_fun, transmission_fun, num_devices, num_nodes * 0.1);
  crono.stop();
  crono.wallTime();


  Chrono crono2;
  crono2.start();
  auto const res2 = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
    weight_fun, transmission_fun, num_devices, num_nodes * 0.1);
  crono2.stop();
  crono2.wallTime();

  for (auto i = 0; i < res.size(); ++i)
    for (auto j = 0; j < res2.size(); ++j)
      if (i != j && !(res[i] < res2[j] || res2[j] < res[i]))
        ASSERT_TRUE(!(res[i] < res2[j] || res2[j] < res[i]));

  std::cout << std::endl;
}