//
// Created by faccus on 12/12/21.
//

#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/Butcher.h"
#include "../src/Helpers/Computer/Computer_time.h"
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

  std::vector<std::function<type_weight(edge_type const &)>>
  basic_weight(Graph<graph_input_type> const &,
               std::size_t,
               std::vector<type_collection_weights> &);

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    basic_transmission(std::size_t, std::size_t);


  std::vector<Hardware_specifications>
  basic_hardware();

  std::vector<std::function<type_weight(edge_type const &)>>
  real_weight(std::vector<type_collection_weights> &,
              Butcher<graph_input_type> &);

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  real_transmission(Butcher<graph_input_type> &);



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

    std::cout << "Average time per test: "
              << total_time / number_of_tests / 1000 << " milli-seconds"
              << std::endl;
  }

  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_eppstein_multiple_random)
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
        auto const res = butcher.compute_k_shortest_paths_eppstein_linear(
          maps, transmission_fun, num_devices, num_nodes * 0.1);
        crono.stop();

        total_time += crono.wallTime();
      }

    std::cout << "Average time per test: "
              << total_time / number_of_tests / 1000 << " milli-seconds"
              << std::endl;
  }

  TEST(ButcherBenchmarkTest,
       compute_k_shortest_paths_eppstein_vs_lazy_random_multiple)
  {
    std::map<io_id_type, Input> map;
    std::vector<node_type>      nodes;

    std::size_t       num_devices = 3;
    std::size_t const num_nodes   = 100;

    std::size_t k = 1000;

    std::size_t number_of_tests = 1000;


    auto        butcher = basic_butcher(num_nodes);
    auto const &graph   = butcher.getGraph();

    double time_std  = .0;
    double time_lazy = .0;

    for (auto num_test = 0; num_test < number_of_tests; ++num_test)
      {
        std::vector<type_collection_weights> weight_maps;
        auto maps = basic_weight(num_devices, num_nodes, weight_maps);

        auto transmission_fun =
          basic_transmission(num_devices, butcher.getGraph().nodes.size());

        Chrono crono;
        crono.start();
        auto eppstein_res = butcher.compute_k_shortest_paths_eppstein_linear(
          maps, transmission_fun, num_devices, k);
        crono.stop();
        time_std += crono.wallTime();

        crono.start();
        auto lazy_eppstein_res =
          butcher.compute_k_shortest_paths_lazy_eppstein_linear(
            maps, transmission_fun, num_devices, k);
        crono.stop();
        time_lazy += crono.wallTime();

        ASSERT_EQ(eppstein_res.size(), lazy_eppstein_res.size());

        auto const last_weight_epp = eppstein_res.back().second;
        ASSERT_EQ(last_weight_epp, lazy_eppstein_res.back().second);

        auto tmp_it  = --eppstein_res.end();
        auto tmp_it2 = --lazy_eppstein_res.end();

        for (; tmp_it != eppstein_res.begin() &&
               tmp_it2 != lazy_eppstein_res.begin();
             --tmp_it, --tmp_it2)
          {
            if (tmp_it->second != last_weight_epp)
              break;
          }

        ++tmp_it;
        ++tmp_it2;

        eppstein_res.erase(tmp_it, eppstein_res.end());
        lazy_eppstein_res.erase(tmp_it2, lazy_eppstein_res.end());

        std::set<
          std::pair<std::vector<std::pair<size_t, std::set<node_id_type>>>,
                    type_weight>>
          eppstein;
        eppstein.insert(eppstein_res.begin(), eppstein_res.end());

        std::set<
          std::pair<std::vector<std::pair<size_t, std::set<node_id_type>>>,
                    type_weight>>
          lazy_eppstein;
        lazy_eppstein.insert(lazy_eppstein_res.begin(),
                             lazy_eppstein_res.end());

        ASSERT_EQ(eppstein, lazy_eppstein);
      }

    std::cout << "Lazy Eppstein: " << time_lazy / 1000 / number_of_tests
              << " milliseconds" << std::endl;

    std::cout << "Eppstein: " << time_std / 1000 / number_of_tests
              << " milliseconds" << std::endl;
  }


  TEST(ButcherBenchmarkTest,
       compute_k_shortest_paths_test_network_basic_weights)
  {
    std::string path      = "resnet18-v2-7-inferred";
    std::string extension = ".onnx";

    Butcher<graph_input_type> butcher(path + extension);

    auto const &graph = butcher.getGraph();
    auto const &nodes = graph.nodes;
    auto const  k     = 1000;

    std::size_t                          num_devices = 3;
    std::vector<type_collection_weights> weight_maps;


    auto maps             = basic_weight(graph, num_devices, weight_maps);
    auto transmission_fun = basic_transmission(num_devices, nodes.size());

    Chrono crono;
    crono.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_paths_lazy_eppstein_linear(maps,
                                                            transmission_fun,
                                                            num_devices,
                                                            k);
    crono.stop();

    std::cout << "Lazy Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;

    crono.start();
    auto eppstein_res = butcher.compute_k_shortest_paths_eppstein_linear(
      maps, transmission_fun, num_devices, k);
    crono.stop();

    std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;


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


    crono.start();
    auto model_decomp =
      butcher.reconstruct_model(lazy_eppstein_res.back().first);
    crono.stop();

    std::cout << "Model reconstruction: " << crono.wallTime() / 1000
              << " milliseconds" << std::endl;

    /*
    crono.start();
    for (std::size_t i = 0; i < model_decomp.size(); ++i)
      {
        auto const &model    = model_decomp[i];
        std::string out_path = path;
        out_path += "-" + std::to_string(i) + "-dev" +
                    std::to_string(model.second) + extension;

        utilities::output_onnx_file(model.first, out_path);
      }
    crono.stop();
    std::cout << "Model output: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;
    */
  }

  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_test_network_real_weights)
  {
    std::string path      = "resnet18-v2-7-inferred";
    std::string extension = ".onnx";

    Butcher<graph_input_type> butcher(path + extension);

    auto const &graph = butcher.getGraph();
    auto const &nodes = graph.nodes;
    auto const  k     = 1000;

    std::size_t                          num_devices = 3;
    std::vector<type_collection_weights> weight_maps;

    auto maps             = real_weight(weight_maps, butcher);
    auto transmission_fun = real_transmission(butcher);

    Chrono crono;
    crono.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_paths_lazy_eppstein_linear(maps,
                                                            transmission_fun,
                                                            num_devices,
                                                            k);
    crono.stop();

    std::cout << "Lazy Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;

    crono.start();
    auto eppstein_res = butcher.compute_k_shortest_paths_eppstein_linear(
      maps, transmission_fun, num_devices, k);
    crono.stop();

    std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;


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


    crono.start();
    auto model_decomp =
      butcher.reconstruct_model(lazy_eppstein_res.back().first);
    crono.stop();

    std::cout << "Model reconstruction: " << crono.wallTime() / 1000
              << " milliseconds" << std::endl;

    /*
    crono.start();
    for (std::size_t i = 0; i < model_decomp.size(); ++i)
      {
        auto const &model    = model_decomp[i];
        std::string out_path = path;
        out_path += "-" + std::to_string(i) + "-dev" +
                    std::to_string(model.second) + extension;

        utilities::output_onnx_file(model.first, out_path);
      }
    crono.stop();
    std::cout << "Model output: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;
    */
  }




  TEST(ButcherBenchmarkTest, correct_basic_weight_generation)
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

  TEST(ButcherBenchmarkTest, correct_real_weight_generation)
  {
    std::vector<type_collection_weights> basic_weight_maps;
    std::vector<type_collection_weights> real_weight_maps;

    std::string path      = "resnet18-v2-7-inferred";
    std::string extension = ".onnx";

    Butcher<graph_input_type> butcher(path + extension);
    std::size_t const         num_devices = 3;
    std::size_t const         num_nodes   = butcher.getGraph().nodes.size();


    basic_weight(num_devices, num_nodes, basic_weight_maps);
    real_weight(real_weight_maps, butcher);

    for (auto const &pair : basic_weight_maps.front())
      for (std::size_t k = 1; k < num_devices; ++k)
        {}
  }


  Butcher<Input>
  basic_butcher(int num_nodes)
  {
    std::vector<node_type>                     nodes;
    std::map<io_id_type, TestMemoryUsage<int>> map;

    nodes.push_back(node_type(io_id_collection_type{}, {{"Y", 0}}));
    for (int n = 1; n < num_nodes - 1; ++n)
      nodes.push_back(node_type({{"X", n - 1}}, {{"Y", n}}));
    nodes.push_back(node_type({{"X", num_nodes - 2}}, {{"Y", 0}}));

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

    std::uniform_int_distribution node_weights_generator{5000, 10000};

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

  std::vector<std::function<type_weight(edge_type const &)>>
  basic_weight(Graph<graph_input_type> const        &graph,
               std::size_t                           num_devices,
               std::vector<type_collection_weights> &weight_maps)
  {
    std::random_device         rd;
    std::default_random_engine random_engine{rd()};

    auto const num_nodes = graph.nodes.size();

    std::uniform_int_distribution node_weights_generator{5000, 10000};

    weight_maps.reserve(num_devices);

    weight_maps.emplace_back();
    auto &initial_weight_map = weight_maps.back();

    for (std::size_t tail = 0; tail < graph.nodes.size(); ++tail)
      for (auto const &head : graph.dependencies[tail].second)
        initial_weight_map[{tail, head}] =
          node_weights_generator(random_engine);

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
              return 1000.; // return 1000.;
            else if (out_device_id - in_device_id == 1)
              {
                if (out_device_id == 2)
                  return 700.; // return 700.;
                else
                  return 300.; // return 300.;
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


  std::vector<Hardware_specifications>
  basic_hardware()
  {
    std::vector<Hardware_specifications> res;
    std::size_t const                    num_devices = 3;

    std::map<std::string, std::pair<time_type, time_type>> map;
    map["conv"]               = {1.83E-1, 3.43E-10};
    map["batchnormalization"] = {1.64E-2, 7.11E-9};
    map["relu"]               = {8.91E-3, 1.17E-8};
    map["maxpool"]            = {1.44E-2, 1.45E-8};

    std::vector<std::string> names = {"NVIDIA Quadro M6002",
                                      "NVIDIA Quadro M6001",
                                      "NVIDIA Quadro M6000"};

    for (int i = num_devices - 1; i >= 0; --i)
      {
        res.emplace_back(names[i]);
        for (auto const &pair : map)
          {
            auto to_insert = pair.second;
            to_insert.first *= std::pow(10, i);
            to_insert.second *= std::pow(10, i);

            res.back().set_regression_coefficient(pair.first, to_insert);
          }
      }

    return res;
  }

  std::vector<std::function<type_weight(edge_type const &)>>
  real_weight(std::vector<type_collection_weights> &weight_maps,
              Butcher<graph_input_type>            &butcher)
  {
    std::vector<std::function<type_weight(edge_type const &)>> res;
    auto             &graph       = butcher.getGraph();
    std::size_t const num_devices = 3;
    weight_maps.reserve(num_devices);

    auto          hws = basic_hardware();
    Computer_time cp;


    for (std::size_t i = 0; i < hws.size(); ++i)
      {
        weight_maps.emplace_back();
        auto &map = weight_maps.back();
        auto &hw  = hws[i];

        for (std::size_t tail = 0; tail < graph.dependencies.size(); ++tail)
          {
            for (auto const &head : graph.dependencies[tail].second)
              map[{tail, head}] = cp.compute_operation_time(graph, tail, hw);
          }
      }

    for (std::size_t i = 0; i < weight_maps.size(); ++i)
      {
        auto &map = weight_maps[i];
        res.emplace_back([&map](edge_type const &edge) { return map[edge]; });
      }

    return res;
  }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  real_transmission(Butcher<graph_input_type> &butcher)
  {
    auto const &graph = butcher.getGraph();
    auto const  mbps  = 1000. / 8;


    return [&graph, mbps](node_id_type const &node,
                          std::size_t         from_device,
                          std::size_t         to_device) {
      Computer_memory cm;
      auto const      mem_to_transmit =
        cm.compute_memory_usage_output(graph, graph.nodes[node], false);

      if (from_device == 0 && to_device == 1 ||
          from_device == 1 && to_device == 0)
        return mem_to_transmit / (18.88 * mbps);
      else if (from_device == 0 && to_device == 2)
        return mem_to_transmit / (5.85 * mbps) +
               mem_to_transmit / (18.88 * mbps);
      else if (from_device == 1 && to_device == 2)
        return mem_to_transmit / (5.85 * mbps);
      else if (from_device == 2 && to_device == 0)
        return mem_to_transmit / (18.88 * mbps) +
               mem_to_transmit / (13.76 * mbps);
      else if (from_device == 2 && to_device == 1)
        return mem_to_transmit / (13.76 * mbps);
      else
        return .0;
    };
  }


}; // namespace butcher_benchmark_test_namespace