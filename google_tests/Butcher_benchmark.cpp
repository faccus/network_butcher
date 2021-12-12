//
// Created by faccus on 12/12/21.
//

#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/Butcher.h"
#include "../src/Helpers/APCS/chrono.h"
#include "TestClass.h"


Butcher<TestMemoryUsage<int>>
basic_butcher(int);

TEST(ButcherBenchmarkTest, compute_k_shortest_paths_lazy_eppstein_random)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::size_t       num_devices = 3;
  std::size_t const num_nodes   = 1000;


  auto        butcher = basic_butcher(num_nodes);
  auto const &graph   = butcher.getGraph();

  std::random_device         rd;
  std::default_random_engine random_engine{rd()};

  std::uniform_real_distribution node_weights_generator{5000., 10000.};


  std::vector<type_collection_weights> weight_maps;
  weight_maps.reserve(num_devices);

  weight_maps.emplace_back();
  auto &initial_weight_map = weight_maps.back();

  for (node_id_type i = 0; i < num_nodes - 1; ++i)
    initial_weight_map[{i, i + 1}] = node_weights_generator(random_engine);

  for (std::size_t k = 1; k < num_devices; ++k)
    {
      weight_maps.emplace_back(weight_maps.front());
      auto &tmp_map = weight_maps.back();
      for (auto &edge : tmp_map)
        edge.second /= std::pow(2, k);
    }

  std::vector<std::function<type_weight(edge_type const &)>> maps;

  for (std::size_t i = 0; i < num_devices; ++i)
    {
      auto &weight_map = weight_maps[i];

      maps.emplace_back([&weight_map](edge_type const &edge) {
        return weight_map.find(edge)->second;
      });
    }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    transmission_fun =
      [&](node_id_type const &input, std::size_t first, std::size_t second) {
        auto in_device_id  = first;
        auto out_device_id = second;

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
      };

  Chrono crono;
  crono.start();
  auto const res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
    maps, transmission_fun, num_devices, num_nodes * 0.1);
  crono.stop();

  crono.wallTime();


  std::cout << std::endl;
}

TEST(ButcherBenchmarkTest,
     compute_k_shortest_paths_lazy_eppstein_multiple_random)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;


  std::size_t       num_devices     = 3;
  std::size_t const num_nodes       = 100;
  std::size_t const number_of_tests = 100;

  auto        butcher = basic_butcher(num_nodes);
  auto const &graph   = butcher.getGraph();

  std::random_device             rd;
  std::default_random_engine     random_engine{rd()};
  std::uniform_real_distribution node_weights_generator{5000., 10000.};

  double total_time = .0;

  for (auto num_test = 0; num_test < number_of_tests; ++num_test)
    {
      std::vector<type_collection_weights> weight_maps;
      weight_maps.reserve(num_devices);

      weight_maps.emplace_back();
      auto &initial_weight_map = weight_maps.back();

      for (node_id_type i = 0; i < num_nodes - 1; ++i)
        initial_weight_map[{i, i + 1}] = node_weights_generator(random_engine);

      for (std::size_t k = 1; k < num_devices; ++k)
        {
          weight_maps.emplace_back(weight_maps.front());
          auto &tmp_map = weight_maps.back();
          for (auto &edge : tmp_map)
            edge.second /= std::pow(2, k);
        }

      std::vector<std::function<type_weight(edge_type const &)>> maps;

      for (std::size_t i = 0; i < num_devices; ++i)
        {
          auto &weight_map = weight_maps[i];

          maps.emplace_back([&weight_map](edge_type const &edge) {
            return weight_map.find(edge)->second;
          });
        }

      std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
        transmission_fun = [&](node_id_type const &input,
                               std::size_t         first,
                               std::size_t         second) {
          auto in_device_id  = first;
          auto out_device_id = second;

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
        };

      Chrono crono;
      crono.start();
      auto const res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
        maps, transmission_fun, num_devices, num_nodes * 0.1);
      crono.stop();

      total_time += crono.wallTime();
    }

  std::cout << "Average time per test: " << total_time / number_of_tests
            << " micro-seconds" << std::endl;
  ASSERT_GE(total_time / number_of_tests, 0);
}

TEST(ButcherBenchmarkTest, compute_k_shortest_paths_eppstein_random)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::size_t       num_devices = 3;
  std::size_t const num_nodes   = 1000;


  auto        butcher = basic_butcher(num_nodes);
  auto const &graph   = butcher.getGraph();

  std::random_device         rd;
  std::default_random_engine random_engine{rd()};
  type_collection_weights    additional_weights;

  std::uniform_real_distribution node_weights_generator{5000., 10000.};


  std::vector<type_collection_weights> weight_maps;
  weight_maps.reserve(num_devices);

  weight_maps.emplace_back();
  auto &initial_weight_map = weight_maps.back();

  for (node_id_type i = 0; i < num_nodes - 1; ++i)
    initial_weight_map[{i, i + 1}] = node_weights_generator(random_engine);

  for (std::size_t k = 1; k < num_devices; ++k)
    {
      weight_maps.emplace_back(weight_maps.front());
      auto &tmp_map = weight_maps.back();
      for (auto &edge : tmp_map)
        edge.second /= std::pow(2, k);
    }

  std::vector<std::function<type_weight(edge_type const &)>> maps;

  for (std::size_t i = 0; i < num_devices; ++i)
    {
      auto &weight_map = weight_maps[i];

      maps.emplace_back([&weight_map](edge_type const &edge) {
        return weight_map.find(edge)->second;
      });
    }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    transmission_fun =
      [&](node_id_type const &input, std::size_t first, std::size_t second) {
        auto in_device_id  = first;
        auto out_device_id = second;

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
      };

  Chrono crono;
  crono.start();
  auto const res = butcher.compute_k_shortest_paths_eppstein_linear(
    maps, transmission_fun, num_devices, num_nodes * 0.1);
  crono.stop();

  crono.wallTime();


  std::cout << std::endl;
}

TEST(ButcherBenchmarkTest, compute_k_shortest_paths_eppstein_vs_lazy_random)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  std::size_t       num_devices = 3;
  std::size_t const num_nodes   = 1000;


  auto        butcher = basic_butcher(num_nodes);
  auto const &graph   = butcher.getGraph();

  std::random_device         rd;
  std::default_random_engine random_engine{rd()};
  type_collection_weights    additional_weights;

  std::uniform_real_distribution node_weights_generator{5000., 10000.};

  std::vector<type_collection_weights> weight_maps;
  weight_maps.reserve(num_devices);

  weight_maps.emplace_back();
  auto &initial_weight_map = weight_maps.back();

  for (node_id_type i = 0; i < num_nodes - 1; ++i)
    initial_weight_map[{i, i + 1}] = node_weights_generator(random_engine);

  for (std::size_t k = 1; k < num_devices; ++k)
    {
      weight_maps.emplace_back(weight_maps.front());
      auto &tmp_map = weight_maps.back();
      for (auto &edge : tmp_map)
        edge.second /= std::pow(2, k);
    }

  std::vector<std::function<type_weight(edge_type const &)>> maps;

  for (std::size_t i = 0; i < num_devices; ++i)
    {
      auto &weight_map = weight_maps[i];

      maps.emplace_back([&weight_map](edge_type const &edge) {
        return weight_map.find(edge)->second;
      });
    }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    transmission_fun =
      [&](node_id_type const &input, std::size_t first, std::size_t second) {
        auto in_device_id  = first;
        auto out_device_id = second;

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
      };

  Chrono crono;
  crono.start();
  auto const tres = butcher.compute_k_shortest_paths_eppstein_linear(
    maps, transmission_fun, num_devices, num_nodes * 0.1);
  crono.stop();


  Chrono crono2;
  crono2.start();
  auto const tres2 = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
    maps, transmission_fun, num_devices, num_nodes * 0.1);
  crono2.stop();
  crono2.wallTime();

  auto const &res  = tres.second;
  auto const &res2 = tres2.second;

  for (auto i = 0; i < res.size(); ++i)
    for (auto j = 0; j < res2.size(); ++j)
      EXPECT_FALSE(i != j && !(res[i] < res2[j] || res2[j] < res[i]));

  std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
            << std::endl;
  std::cout << "Lazy eppstein: " << crono2.wallTime() / 1000 << " milliseconds"
            << std::endl;
}


Butcher<TestMemoryUsage<int>>
basic_butcher(int num_nodes)
{
  std::vector<node_type>                     nodes;
  std::map<io_id_type, TestMemoryUsage<int>> map;

  nodes.push_back(node_type(io_id_collection_type{}, {0}));
  for (int n = 1; n < num_nodes - 1; ++n)
    nodes.push_back(node_type(io_id_collection_type{n - 1}, {n}));
  nodes.push_back(node_type(io_id_collection_type{num_nodes - 2}, {0}));

  Graph<TestMemoryUsage<int>> graph_cons(nodes, map);

  return Butcher(std::move(graph_cons));
}