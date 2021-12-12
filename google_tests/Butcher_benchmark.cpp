//
// Created by faccus on 12/12/21.
//

#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/Butcher.h"
#include "../src/Helpers/APCS/chrono.h"
#include "TestClass.h"

namespace butcher_benchmark_test_namespace
{

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  using basic_type = int;
  using Input      = TestMemoryUsage<int>;

  Butcher<Input>
  basic_butcher(int);

  std::vector<std::function<type_weight(edge_type const &)>>
  basic_weight(std::size_t,
               std::size_t,
               std::vector<type_collection_weights> &);

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    basic_transmission(std::size_t, std::size_t);


  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_lazy_eppstein_random)
  {
    std::size_t       num_devices = 3;
    std::size_t const num_nodes   = 1000;


    auto        butcher = basic_butcher(num_nodes);
    auto const &graph   = butcher.getGraph();

    std::vector<type_collection_weights> weight_maps;
    auto maps = basic_weight(num_devices, num_nodes, weight_maps);

    auto transmission_fun =
      basic_transmission(num_devices, butcher.getGraph().nodes.size());

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
    std::size_t       num_devices     = 3;
    std::size_t const num_nodes       = 100;
    std::size_t const number_of_tests = 100;

    auto        butcher = basic_butcher(num_nodes);
    auto const &graph   = butcher.getGraph();

    double total_time = .0;

    for (auto num_test = 0; num_test < number_of_tests; ++num_test)
      {
        std::vector<type_collection_weights> weight_maps;
        auto maps = basic_weight(num_devices, num_nodes, weight_maps);

        auto transmission_fun =
          basic_transmission(num_devices, butcher.getGraph().nodes.size());

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
    std::size_t       num_devices = 3;
    std::size_t const num_nodes   = 1000;


    auto        butcher = basic_butcher(num_nodes);
    auto const &graph   = butcher.getGraph();

    std::vector<type_collection_weights> weight_maps;
    auto maps = basic_weight(num_devices, num_nodes, weight_maps);

    auto transmission_fun =
      basic_transmission(num_devices, butcher.getGraph().nodes.size());

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
    std::map<io_id_type, Input> map;
    std::vector<node_type>      nodes;

    std::size_t       num_devices = 3;
    std::size_t const num_nodes   = 1000;


    auto        butcher = basic_butcher(num_nodes);
    auto const &graph   = butcher.getGraph();


    std::vector<type_collection_weights> weight_maps;
    auto maps = basic_weight(num_devices, num_nodes, weight_maps);

    auto transmission_fun =
      basic_transmission(num_devices, butcher.getGraph().nodes.size());

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

    ASSERT_EQ(res.size(), res2.size());

    for (auto i = 0; i < res.size(); ++i)
      {
        auto const &first  = res[i];
        auto const &second = res2[i];

        EXPECT_EQ(first.path, second.path);
        EXPECT_DOUBLE_EQ(first.length, second.length);
      }

    std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;
    std::cout << "Lazy eppstein: " << crono2.wallTime() / 1000
              << " milliseconds" << std::endl;
  }


  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_test_network)
  {
    std::string path = "resnet18-v2-7-inferred.onnx";

    Butcher<graph_input_type> butcher(path);

    auto const &graph     = butcher.getGraph();
    auto const &nodes     = graph.nodes;
    auto const  num_nodes = nodes.size();

    std::size_t                          num_devices = 3;
    std::vector<type_collection_weights> weight_maps;

    auto maps             = basic_weight(num_devices, num_nodes, weight_maps);
    auto transmission_fun = basic_transmission(num_devices, num_nodes);

    Chrono crono;
    crono.start();
    auto const tmp_res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
      maps, transmission_fun, num_devices, num_nodes / 10);
    crono.stop();

    std::cout << "Lazy Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;

    crono.start();
    auto res = butcher.reconstruct_model(tmp_res.first, tmp_res.second.back());
    crono.stop();

    std::cout << "Model reconstruction: " << crono.wallTime() / 1000
              << " milliseconds" << std::endl;
  }


  TEST(ButcherBenchmarkTest, correct_weight_generation)
  {
    std::vector<type_collection_weights> weight_maps;


    std::size_t num_nodes   = 100;
    std::size_t num_devices = 3;

    auto maps = basic_weight(num_devices, num_nodes, weight_maps);

    for (auto const &pair : weight_maps.front())
      for (std::size_t k = 1; k < num_devices; ++k)
        {
          auto const second_value =
            weight_maps[k].find(pair.first)->second * std::pow(2, k);

          EXPECT_DOUBLE_EQ(pair.second, second_value);
        }
  }

  Butcher<Input>
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

  std::vector<std::function<type_weight(edge_type const &)>>
  basic_weight(std::size_t                           num_devices,
               std::size_t                           num_nodes,
               std::vector<type_collection_weights> &weight_maps)
  {
    std::random_device         rd;
    std::default_random_engine random_engine{rd()};

    std::uniform_real_distribution node_weights_generator{5000., 10000.};

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

    return maps;
  }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  basic_transmission(std::size_t devices, std::size_t size)
  {
    return
      [devices,
       size](node_id_type const &input, std::size_t first, std::size_t second) {
        if (0 <= input && input < size && 0 <= first < devices && 0 <= second &&
            second < devices)
          {
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
          }
        else
          {
            std::cout << "Incorrect device id or input id" << std::endl;
            return -1.;
          }
      };
  }
}; // namespace butcher_benchmark_test_namespace