//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/Butcher.h"
#include "../src/Helpers/IO_Manager.h"
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
  using Content_type = Content<Input>;
  using Node_type = Node<Content_type>;

  Butcher<Content_type>
  basic_butcher();

  std::vector<std::function<type_weight(edge_type const &)>>
  basic_weight(std::size_t, std::vector<type_collection_weights> &);

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    basic_transmission(std::size_t, std::size_t);


  TEST(ButcherTest, compute_two_slice_brute_force_test)
  {
    using Content_type = graph_input_type;

    const std::string model_path = "resnet18-v2-7-inferred.onnx";

    Butcher<Content_type> butcher(IO_Manager::import_from_onnx(model_path).first);
    auto           res = butcher.compute_two_slice_brute_force();
  }

  TEST(ButcherTest, compute_two_slice_memory_brute_force_test)
  {
    using Content_type = graph_input_type;

    const std::string model_path = "resnet18-v2-7-inferred.onnx";

    Graph<Content_type> graph = IO_Manager::import_from_onnx(model_path).first;
    const Computer_memory computer{};

    size_t half_size = computer.compute_memory_usage_input(graph) / 2;

    Butcher<Content_type> butcher(std::move(graph));

    auto tot = butcher.compute_two_slice_memory_brute_force(half_size);
  }

  TEST(ButcherTest, compute_k_shortest_paths_eppstein_linear)
  {
    std::size_t num_devices = 3;

    auto                                 butcher = basic_butcher();
    std::vector<type_collection_weights> weight_maps;

    auto const &graph     = butcher.get_graph();
    auto const  num_nodes = graph.get_nodes().size();

    auto maps = basic_weight(num_devices, weight_maps);
    auto transmission_fun = basic_transmission(num_devices, num_nodes);

    auto const res = butcher.compute_k_shortest_paths_eppstein_linear(
      maps, transmission_fun, num_devices, 1000);

    ASSERT_EQ(res.size(), 81);
  }

  TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_linear)
  {
    std::size_t num_devices = 3;

    auto                                 butcher = basic_butcher();
    std::vector<type_collection_weights> weight_maps;

    auto maps = basic_weight(num_devices, weight_maps);
    auto const &graph     = butcher.get_graph();
    auto const  num_nodes = graph.get_nodes().size();

    auto transmission_fun = basic_transmission(num_devices, num_nodes);

    auto const res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
      maps, transmission_fun, num_devices, 1000);

    ASSERT_EQ(res.size(), 81);
  }

  TEST(ButcherTest, compute_k_shortest_paths_eppstein_vs_lazy_random)
  {
    std::size_t num_devices = 3;
    std::size_t k           = 1000;


    auto        butcher = basic_butcher();
    auto const &graph   = butcher.get_graph();
    auto const &nodes = graph.get_nodes();

    std::vector<type_collection_weights> weight_maps;
    auto maps = basic_weight(num_devices, weight_maps);

    auto transmission_fun =
      basic_transmission(num_devices, nodes.size());

    Chrono crono;
    crono.start();
    auto eppstein_res = butcher.compute_k_shortest_paths_eppstein_linear(
      maps, transmission_fun, num_devices, k);
    crono.stop();


    Chrono crono2;
    crono2.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_paths_lazy_eppstein_linear(maps,
                                                            transmission_fun,
                                                            num_devices,
                                                            k);
    crono2.stop();
    crono2.wallTime();

    ASSERT_EQ(eppstein_res.size(), lazy_eppstein_res.size());

    auto const last_weight_epp = eppstein_res.back().second;
    ASSERT_EQ(last_weight_epp, lazy_eppstein_res.back().second);

    auto tmp_it  = --eppstein_res.end();
    auto tmp_it2 = --lazy_eppstein_res.end();

    for (;
         tmp_it != eppstein_res.begin() && tmp_it2 != lazy_eppstein_res.begin();
         --tmp_it, --tmp_it2)
      {
        if (tmp_it->second != last_weight_epp)
          break;
      }

    ++tmp_it;
    ++tmp_it2;

    eppstein_res.erase(tmp_it, eppstein_res.end());
    lazy_eppstein_res.erase(tmp_it2, lazy_eppstein_res.end());


    std::set<std::pair<std::vector<std::pair<size_t, std::set<node_id_type>>>,
                       type_weight>>
      eppstein;
    eppstein.insert(eppstein_res.begin(), eppstein_res.end());

    std::set<std::pair<std::vector<std::pair<size_t, std::set<node_id_type>>>,
                       type_weight>>
      lazy_eppstein;
    lazy_eppstein.insert(lazy_eppstein_res.begin(), lazy_eppstein_res.end());

    ASSERT_EQ(eppstein, lazy_eppstein);


    std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;
    std::cout << "Lazy eppstein: " << crono2.wallTime() / 1000
              << " milliseconds" << std::endl;
  }


  Butcher<Content_type>
  basic_butcher()
  {
    std::vector<Node_type>      nodes;

    nodes.emplace_back(Content_type({}, {{"X0", 0}}));
    nodes.emplace_back(Content_type({{"X0", 0}}, {{"X1", 1}}));
    nodes.emplace_back(Content_type({{"X1", 1}}, {{"X2", 2}}));
    nodes.emplace_back(Content_type({{"X1", 1}}, {{"X3", 3}}));
    nodes.emplace_back(Content_type({{"X3", 3}}, {{"X4", 4}}));
    nodes.emplace_back(Content_type({{"X2", 2}, {"X4", 4}}, {{"X5", 5}}));
    nodes.emplace_back(Content_type({{"X5", 5}}, {{"X6", 6}}));
    nodes.emplace_back(Content_type({{"X6", 6}}, {{"X7", 7}}));

    Graph graph_cons(std::move(nodes));
    return Butcher(std::move(graph_cons));
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