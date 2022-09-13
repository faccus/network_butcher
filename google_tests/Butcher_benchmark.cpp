//
// Created by faccus on 12/12/21.
//

#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../include/APSC/chrono.h"
#include "../include/Butcher.h"
#include "../include/Computer/Computer_time.h"
#include "../include/IO_Manager.h"
#include "TestClass.h"

namespace
{
  using namespace network_butcher_computer;
  using namespace network_butcher_types;

  using type_weight             = double;
  using type_collection_weights = std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  using basic_type   = int;
  using Input        = TestMemoryUsage<int>;
  using Content_type = Content<Input>;
  using Node_type    = Node<Content_type>;

  using Graph_type      = MWGraph<Content_type>;
  using Real_Graph_Type = MWGraph<graph_input_type>;


  Butcher<Graph_type>
  basic_butcher(int);

  std::tuple<Butcher<graph_type>, onnx::ModelProto, std::map<node_id_type, node_id_type>>
  real_butcher();

  Parameters
  eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices);

  Parameters
  lazy_eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices);
  Parameters
  real_parameters(std::size_t k, bool backward);

  template <class Graph>
  void
  complete_weights(Graph &graph)
  {
    auto const  num_nodes    = graph.get_nodes().size();
    auto const &dependencies = graph.get_dependencies();

    for (node_id_type tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : dependencies[tail].second)
        {
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
            {
              if (graph.get_weigth(k, {tail, head}) == -1.)
                graph.set_weigth(k, {tail, head}, 0.);
            }
        }
  };

  template <class Graph>
  void
  basic_weight(Graph &graph, bool fully_random)
  {
    std::size_t seed;

    if (fully_random)
      {
        std::random_device rd;
        seed = rd();
      }
    else
      {
        seed = 0;
      }
    std::default_random_engine random_engine{seed};

    std::uniform_int_distribution node_weights_generator{5000, 10000};

    for (std::size_t tail = 0; tail < graph.get_nodes().size(); ++tail)
      for (auto const &head : graph.get_dependencies()[tail].second)
        {
          auto const tmp_weight = node_weights_generator(random_engine);
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
            {
              graph.set_weigth(k, {tail, head}, tmp_weight / std::pow(2, k));
            }
        }
  }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)> basic_transmission(std::size_t,
                                                                                                std::size_t);


  std::vector<Hardware_specifications>
  basic_hardware();

  void
  real_weight(Real_Graph_Type &);

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  real_transmission(Real_Graph_Type const &);


  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_test_network_basic_weights)
  {
    std::string path      = "resnet18-v2-7-inferred";
    std::string extension = ".onnx";

    Butcher butcher(std::get<0>(network_butcher_io::IO_Manager::import_from_onnx(path + extension)));

    auto       &graph = butcher.get_graph_ref();
    auto const &nodes = graph.get_nodes();
    auto const  k     = 1000;

    std::size_t                          num_devices = 3;
    std::vector<type_collection_weights> weight_maps;


    basic_weight(graph, true);
    auto transmission_fun = basic_transmission(num_devices, nodes.size());

    Chrono crono;
    crono.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_path(transmission_fun, lazy_eppstein_parameters(k, true, num_devices));
    crono.stop();

    std::cout << "Lazy Eppstein: " << crono.wallTime() / 1000 << " milliseconds" << std::endl;

    crono.start();
    auto eppstein_res = butcher.compute_k_shortest_path(transmission_fun, eppstein_parameters(k, true, num_devices));
    crono.stop();

    std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds" << std::endl;


    ASSERT_EQ(eppstein_res.size(), lazy_eppstein_res.size());

    auto const last_weight_epp = eppstein_res.back().first;
    ASSERT_EQ(last_weight_epp, lazy_eppstein_res.back().first);

    auto tmp_it  = --eppstein_res.end();
    auto tmp_it2 = --lazy_eppstein_res.end();

    for (; tmp_it != eppstein_res.begin() && tmp_it2 != lazy_eppstein_res.begin(); --tmp_it, --tmp_it2)
      {
        if (tmp_it->first != last_weight_epp)
          break;
      }

    ++tmp_it;
    ++tmp_it2;

    eppstein_res.erase(tmp_it, eppstein_res.end());
    lazy_eppstein_res.erase(tmp_it2, lazy_eppstein_res.end());


    std::set<Weighted_Real_Path> eppstein;
    eppstein.insert(eppstein_res.begin(), eppstein_res.end());

    std::set<Weighted_Real_Path> lazy_eppstein;
    lazy_eppstein.insert(lazy_eppstein_res.begin(), lazy_eppstein_res.end());

    ASSERT_EQ(eppstein, lazy_eppstein);
  }


  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_test_network_real_weights)
  {
    std::string path        = "resnet18-v2-7-inferred";
    std::string extension   = ".onnx";
    std::size_t num_devices = 3;

    Butcher butcher(std::get<0>(network_butcher_io::IO_Manager::import_from_onnx(path + extension, true, num_devices)));

    auto       &graph = butcher.get_graph_ref();
    auto const &nodes = graph.get_nodes();
    auto const  k     = 1000;

    std::vector<type_collection_weights> weight_maps;

    real_weight(graph);
    auto transmission_fun = real_transmission(butcher.get_graph());

    Chrono crono;
    crono.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_path(transmission_fun, lazy_eppstein_parameters(k, true, num_devices));
    crono.stop();

    std::cout << "Lazy Eppstein: " << crono.wallTime() / 1000 << " milliseconds" << std::endl;

    crono.start();
    auto eppstein_res = butcher.compute_k_shortest_path(transmission_fun, eppstein_parameters(k, true, num_devices));
    crono.stop();

    std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds" << std::endl;


    ASSERT_EQ(eppstein_res.size(), lazy_eppstein_res.size());

    auto const last_weight_epp = eppstein_res.back().first;
    ASSERT_EQ(last_weight_epp, lazy_eppstein_res.back().first);

    auto tmp_it  = --eppstein_res.end();
    auto tmp_it2 = --lazy_eppstein_res.end();

    for (; tmp_it != eppstein_res.begin() && tmp_it2 != lazy_eppstein_res.begin(); --tmp_it, --tmp_it2)
      {
        if (tmp_it->first != last_weight_epp)
          break;
      }

    ++tmp_it;
    ++tmp_it2;

    eppstein_res.erase(tmp_it, eppstein_res.end());
    lazy_eppstein_res.erase(tmp_it2, lazy_eppstein_res.end());


    std::set<Weighted_Real_Path> eppstein;
    eppstein.insert(eppstein_res.begin(), eppstein_res.end());

    std::set<Weighted_Real_Path> lazy_eppstein;
    lazy_eppstein.insert(lazy_eppstein_res.begin(), lazy_eppstein_res.end());

    ASSERT_EQ(eppstein, lazy_eppstein);
  }


  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_eppstein_vs_lazy_deterministic_multiple)
  {
    std::vector<node_type> nodes;

    std::size_t       num_devices = 3;
    std::size_t const num_nodes   = 1000;

    std::size_t k = 1000;

    std::size_t number_of_tests = 1000;


    auto  butcher = basic_butcher(num_nodes);
    auto &graph   = butcher.get_graph_ref();

    double time_std  = .0;
    double time_lazy = .0;

    for (auto num_test = 0; num_test < number_of_tests; ++num_test)
      {
        std::vector<type_collection_weights> weight_maps;
        basic_weight(graph, true);

        auto transmission_fun = basic_transmission(num_devices, graph.get_nodes().size());

        Chrono crono;
        crono.start();
        auto eppstein_res =
          butcher.compute_k_shortest_path(transmission_fun, eppstein_parameters(k, true, num_devices));
        crono.stop();
        double const time_instance_std = crono.wallTime();
        time_std += time_instance_std;

        crono.start();
        auto lazy_eppstein_res =
          butcher.compute_k_shortest_path(transmission_fun, lazy_eppstein_parameters(k, true, num_devices));
        crono.stop();
        double const time_instance_lazy = crono.wallTime();
        time_lazy += time_instance_lazy;

        ASSERT_EQ(eppstein_res.size(), lazy_eppstein_res.size());

        auto const last_weight_epp = eppstein_res.back().first;
        ASSERT_EQ(last_weight_epp, lazy_eppstein_res.back().first);

        auto tmp_it  = --eppstein_res.end();
        auto tmp_it2 = --lazy_eppstein_res.end();

        for (; tmp_it != eppstein_res.begin() && tmp_it2 != lazy_eppstein_res.begin(); --tmp_it, --tmp_it2)
          {
            if (tmp_it->first != last_weight_epp)
              break;
          }

        ++tmp_it;
        ++tmp_it2;

        eppstein_res.erase(tmp_it, eppstein_res.end());
        lazy_eppstein_res.erase(tmp_it2, lazy_eppstein_res.end());

        std::set<Weighted_Real_Path> eppstein;
        eppstein.insert(eppstein_res.begin(), eppstein_res.end());

        std::set<Weighted_Real_Path> lazy_eppstein;
        lazy_eppstein.insert(lazy_eppstein_res.begin(), lazy_eppstein_res.end());

        ASSERT_EQ(eppstein, lazy_eppstein);

        std::cout << "Test number #" << (num_test + 1) << ", Lazy: " << time_instance_lazy / 1000
                  << " ms, Epp: " << time_instance_std / 1000 << " ms" << std::endl;
      }

    std::cout << "Lazy Eppstein: " << time_lazy / 1000 / number_of_tests << " milliseconds" << std::endl;

    std::cout << "Eppstein: " << time_std / 1000 / number_of_tests << " milliseconds" << std::endl;
  }


  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_lazy_eppstein_multiple_random)
  {
    std::size_t       num_devices     = 3;
    std::size_t const num_nodes       = 100;
    std::size_t const number_of_tests = 1000;

    auto  butcher = basic_butcher(num_nodes);
    auto &graph   = butcher.get_graph_ref();

    double total_time = .0;

    for (auto num_test = 0; num_test < number_of_tests; ++num_test)
      {
        std::vector<type_collection_weights> weight_maps;
        basic_weight(graph, true);

        auto transmission_fun = basic_transmission(num_devices, graph.get_nodes().size());

        Chrono crono;
        crono.start();
        auto const res = butcher.compute_k_shortest_path(transmission_fun,
                                                         lazy_eppstein_parameters(num_nodes * 0.1, true, num_devices));
        crono.stop();

        total_time += crono.wallTime();
      }

    std::cout << "Average time per test: " << total_time / number_of_tests / 1000 << " milli-seconds" << std::endl;
  }


  TEST(ButcherBenchmarkTest, compute_k_shortest_paths_eppstein_multiple_random)
  {
    std::size_t       num_devices     = 3;
    std::size_t const num_nodes       = 100;
    std::size_t const number_of_tests = 1000;

    auto  butcher = basic_butcher(num_nodes);
    auto &graph   = butcher.get_graph_ref();

    double total_time = .0;

    for (auto num_test = 0; num_test < number_of_tests; ++num_test)
      {
        std::vector<type_collection_weights> weight_maps;
        basic_weight(graph, true);

        auto transmission_fun = basic_transmission(num_devices, graph.get_nodes().size());

        Chrono crono;
        crono.start();
        auto const res =
          butcher.compute_k_shortest_path(transmission_fun, eppstein_parameters(num_nodes * 0.1, true, num_devices));
        crono.stop();

        total_time += crono.wallTime();
      }

    std::cout << "Average time per test: " << total_time / number_of_tests / 1000 << " milli-seconds" << std::endl;
  }


  Butcher<Graph_type>
  basic_butcher(int num_nodes)
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(Content_type({}, {{"X0", 0}}));
    for (int n = 1; n < num_nodes - 1; ++n)
      nodes.emplace_back(Content_type({{"X" + std::to_string(n - 1), n - 1}}, {{"X" + std::to_string(n), n}}));
    nodes.emplace_back(Content_type({{"X" + std::to_string(num_nodes - 2), num_nodes - 2}}, {}));

    Graph_type graph_cons(3, std::move(nodes));

    return Butcher(std::move(graph_cons));
  }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  basic_transmission(std::size_t devices, std::size_t size)
  {
    return [devices, size](node_id_type const &input, std::size_t first, std::size_t second) {
      if (0 <= input && input < size && first < devices && second < devices)
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

    std::vector<std::string> names = {"NVIDIA Quadro M6002", "NVIDIA Quadro M6001", "NVIDIA Quadro M6000"};

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


  void
  real_weight(Real_Graph_Type &graph)
  {
    std::vector<std::function<type_weight(edge_type const &)>> res;
    auto const                                                &dependencies = graph.get_dependencies();

    auto          hws = basic_hardware();
    Computer_time cp;


    for (std::size_t i = 0; i < hws.size(); ++i)
      {
        for (std::size_t tail = 0; tail < dependencies.size(); ++tail)
          {
            for (auto const &head : dependencies[tail].second)
              graph.set_weigth(i, {tail, head}, cp.compute_operation_time(graph, tail, hws[i]));
          }
      }
  }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  real_transmission(Real_Graph_Type const &graph)
  {
    auto const mbps = 1000. / 8;

    return [&graph, mbps](node_id_type const &node, std::size_t from_device, std::size_t to_device) {
      Computer_memory cm;
      auto const      mem_to_transmit = cm.compute_memory_usage_output(graph.get_nodes()[node]);

      auto const first  = std::min(from_device, to_device);
      auto const second = std::max(from_device, to_device);

      if (first == 0 && second == 1)
        return mem_to_transmit / (18.88 * mbps);
      else if (first == 0 && second == 2)
        return mem_to_transmit / (5.85 * mbps) + mem_to_transmit / (18.88 * mbps);
      else if (first == 1 && second == 2)
        return mem_to_transmit / (5.85 * mbps);
      else
        return .0;
    };
  }


  std::tuple<Butcher<graph_type>, onnx::ModelProto, std::map<node_id_type, node_id_type>>
  real_butcher()
  {
    std::string const path  = "version-RFB-640-inferred.onnx"; //"version-RFB-640.onnx";
    auto              tuple = network_butcher_io::IO_Manager::import_from_onnx(path, true, 3);
    auto             &graph = std::get<0>(tuple);

    network_butcher_io::IO_Manager::import_weights(Weight_Import_Mode::aMLLibrary,
                                                   graph,
                                                   "aMLLibrary_prediction_pi.csv",
                                                   0);

    network_butcher_io::IO_Manager::import_weights(Weight_Import_Mode::aMLLibrary,
                                                   graph,
                                                   "aMLLibrary_prediction_tegra.csv",
                                                   1);

    network_butcher_io::IO_Manager::import_weights(Weight_Import_Mode::aMLLibrary,
                                                   graph,
                                                   "aMLLibrary_prediction_tegra.csv",
                                                   2);

    complete_weights(graph);

    return {Butcher(graph), std::get<1>(tuple), std::get<2>(tuple)};
  }

  Parameters
  eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices)
  {
    Parameters res;
    res.K                            = k;
    res.backward_connections_allowed = backward;
    res.method                       = KSP_Method::Eppstein;
    res.devices                      = std::vector<Device>(num_devices);
    res.memory_constraint_type       = Memory_Constraint_Type::None;
    res.starting_device_id           = 0;
    res.ending_device_id             = 0;

    return res;
  }

  Parameters
  lazy_eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices)
  {
    Parameters res;
    res.K                            = k;
    res.backward_connections_allowed = backward;
    res.method                       = KSP_Method::Lazy_Eppstein;
    res.devices                      = std::vector<Device>(num_devices);
    res.memory_constraint_type       = Memory_Constraint_Type::None;
    res.starting_device_id           = 0;
    res.ending_device_id             = 0;

    return res;
  }

}; // namespace