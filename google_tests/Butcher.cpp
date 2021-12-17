//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/Butcher.h"
#include "../src/Helpers/APCS/chrono.h"
#include "TestClass.h"

/*
void
Analyze(const onnx::ValueInfoProto &);
void
Analyze(const onnx::NodeProto &);
void
PrintInputOutput(const onnx::ModelProto &);
*/

namespace butcher_test_namespace
{
  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  using basic_type = int;
  using Input      = TestMemoryUsage<int>;

  Butcher<Input>
  basic_butcher();

  std::vector<std::function<type_weight(edge_type const &)>>
  basic_weight(std::size_t, std::vector<type_collection_weights> &);

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    basic_transmission(std::size_t, std::size_t);


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
    std::size_t num_devices = 3;

    auto                                 butcher = basic_butcher();
    std::vector<type_collection_weights> weight_maps;


    auto const &graph     = butcher.getGraph();
    auto const  num_nodes = graph.nodes.size();


    auto maps = basic_weight(num_devices, weight_maps);

    auto transmission_fun =
      basic_transmission(num_devices, butcher.getGraph().nodes.size());


    auto const tmp_res = butcher.compute_k_shortest_paths_eppstein_linear(
      maps, transmission_fun, num_devices, 1000);

    auto const &res = tmp_res.second;

    for (auto i = 0; i < res.size(); ++i)
      for (auto j = 0; j < res.size(); ++j)
        if (i != j && !(res[i] < res[j] || res[j] < res[i]))
          ASSERT_TRUE(!(res[i] < res[j] || res[j] < res[i]));

    ASSERT_EQ(res.size(), 81);
  }

  TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_linear)
  {
    std::size_t num_devices = 3;

    auto                                 butcher = basic_butcher();
    std::vector<type_collection_weights> weight_maps;

    auto maps = basic_weight(num_devices, weight_maps);

    auto transmission_fun =
      basic_transmission(num_devices, butcher.getGraph().nodes.size());


    auto const tmp_res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
      maps, transmission_fun, num_devices, 1000);

    auto const &res = tmp_res.second;

    for (auto i = 0; i < res.size(); ++i)
      for (auto j = 0; j < res.size(); ++j)
        if (i != j && !(res[i] < res[j] || res[j] < res[i]))
          ASSERT_TRUE(!(res[i] < res[j] || res[j] < res[i]));

    ASSERT_EQ(res.size(), 81);
  }

  TEST(ButcherTest, compute_k_shortest_paths_eppstein_vs_lazy_random)
  {
    std::map<io_id_type, Input> map;
    std::vector<node_type>      nodes;

    std::size_t num_devices = 3;
    std::size_t k           = 1000;


    auto        butcher = basic_butcher();
    auto const &graph   = butcher.getGraph();


    std::vector<type_collection_weights> weight_maps;
    auto maps = basic_weight(num_devices, weight_maps);

    auto transmission_fun =
      basic_transmission(num_devices, butcher.getGraph().nodes.size());

    Chrono crono;
    crono.start();
    auto const tres = butcher.compute_k_shortest_paths_eppstein_linear(
      maps, transmission_fun, num_devices, k);
    crono.stop();


    Chrono crono2;
    crono2.start();
    auto const tres2 = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
      maps, transmission_fun, num_devices, k);
    crono2.stop();
    crono2.wallTime();

    ASSERT_EQ(tres.second.size(), tres2.second.size());

    std::set<path_info> eppstein_result;
    eppstein_result.insert(tres.second.begin(), tres.second.end());

    std::set<path_info> lazy_eppstein_result;
    lazy_eppstein_result.insert(tres2.second.begin(), tres2.second.end());


    for (auto it1 = eppstein_result.cbegin(),
              it2 = lazy_eppstein_result.cbegin();
         it1 != eppstein_result.cend() && it2 != lazy_eppstein_result.cend();
         ++it1, ++it2)
      {
        auto const &eppstein      = *it1;
        auto const &lazy_eppstein = *it2;

        EXPECT_EQ(eppstein.path, lazy_eppstein.path);
        EXPECT_DOUBLE_EQ(eppstein.length, lazy_eppstein.length);
      }

    std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;
    std::cout << "Lazy eppstein: " << crono2.wallTime() / 1000
              << " milliseconds" << std::endl;
  }

  Butcher<Input>
  basic_butcher()
  {
    std::map<io_id_type, Input> map;
    std::vector<node_type>      nodes;

    nodes.push_back(node_type(io_id_collection_type{}, {0}));
    nodes.push_back(node_type(io_id_collection_type{0}, {1}));
    nodes.push_back(node_type(io_id_collection_type{1}, {2}));
    nodes.push_back(node_type(io_id_collection_type{1}, {3}));
    nodes.push_back(node_type(io_id_collection_type{3}, {4}));
    nodes.push_back(node_type(io_id_collection_type{2, 4}, {5}));
    nodes.push_back(node_type(io_id_collection_type{5}, {6}));
    nodes.push_back(node_type(io_id_collection_type{6}, {7}));

    for (io_id_type i = 0; i < nodes.size(); ++i)
      map[i] = i;

    Graph<Input> graph_cons(nodes, map);
    return Butcher<Input>(std::move(graph_cons));
  }

  std::vector<std::function<type_weight(edge_type const &)>>
  basic_weight(std::size_t                           num_devices,
               std::vector<type_collection_weights> &weight_maps)
  {
    std::vector<std::function<type_weight(edge_type const &)>> maps;
    weight_maps.reserve(num_devices);

    weight_maps.emplace_back();
    auto &initial_map = weight_maps.back();

    initial_map[{0, 1}] = 1000.;
    initial_map[{1, 2}] = 1000.;
    initial_map[{1, 3}] = 500.;
    initial_map[{3, 4}] = 500.;
    initial_map[{2, 5}] = 1000.;
    initial_map[{4, 5}] = 1000.;
    initial_map[{5, 6}] = 1000.;
    initial_map[{6, 7}] = 0.;

    for (std::size_t k = 1; k < num_devices; ++k)
      {
        weight_maps.emplace_back(weight_maps.front());
        auto &tmp_map = weight_maps.back();
        for (auto &edge : tmp_map)
          edge.second /= std::pow(2, k);
      }

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
} // namespace butcher_test_namespace