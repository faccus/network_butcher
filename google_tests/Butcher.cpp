//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../include/Butcher.h"
#include "../include/Helpers/APSC/chrono.h"
#include "../include/Helpers/IO_Manager.h"
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

  using basic_type   = int;
  using Input        = TestMemoryUsage<int>;
  using Content_type = Content<Input>;
  using Node_type    = Node<Content_type>;
  using GraphType    = MWGraph<Content_type>;

  Butcher<GraphType>
  basic_butcher();

  std::tuple<Butcher<graph_type>,
             onnx::ModelProto,
             std::map<node_id_type, node_id_type>>
  real_butcher();

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

    auto const num_nodes = graph.get_nodes().size();

    std::uniform_int_distribution node_weights_generator{5000, 10000};

    for (std::size_t tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : graph.get_dependencies()[tail].second)
        {
          auto const tmp_weight = node_weights_generator(random_engine);
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
            graph.set_weigth(k, {tail, head}, tmp_weight / std::pow(2, k));
        }
  };

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    basic_transmission(std::size_t, std::size_t);


  TEST(ButcherTest, compute_k_shortest_paths_eppstein_linear)
  {
    std::size_t num_devices = 3;

    auto butcher = basic_butcher();

    auto const &graph     = butcher.get_graph();
    auto const  num_nodes = graph.get_nodes().size();

    auto const res = butcher.compute_k_shortest_paths_eppstein_linear(
      basic_transmission(graph.get_num_devices(), num_nodes), 1000);

    ASSERT_EQ(res.size(), 81);
  }

  TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_linear)
  {
    std::size_t num_devices = 3;

    auto                                 butcher = basic_butcher();

    auto &graph     = butcher.get_graph_ref();
    auto const  num_nodes = graph.get_nodes().size();

    auto transmission_fun = basic_transmission(num_devices, num_nodes);

        auto const res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
          transmission_fun, 1000);

    ASSERT_EQ(res.size(), 81);
  }

  TEST(ButcherTest, compute_k_shortest_paths_eppstein_vs_lazy_linear)
  {
    std::size_t num_devices = 3;
    std::size_t k           = 1000;


    auto        butcher = basic_butcher();
    auto       &graph   = butcher.get_graph_ref();
    auto const &nodes   = graph.get_nodes();

    auto transmission_fun = basic_transmission(num_devices, nodes.size());

    Chrono crono;
    crono.start();
    auto eppstein_res =
      butcher.compute_k_shortest_paths_eppstein_linear(transmission_fun, k);
    crono.stop();


    Chrono crono2;
    crono2.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_paths_lazy_eppstein_linear(transmission_fun,
                                                            k);
    crono2.stop();
    crono2.wallTime();

    ASSERT_EQ(eppstein_res.size(), lazy_eppstein_res.size());

    auto const last_weight_epp = eppstein_res.back().first;
    ASSERT_EQ(last_weight_epp, lazy_eppstein_res.back().first);

    auto tmp_it  = --eppstein_res.end();
    auto tmp_it2 = --lazy_eppstein_res.end();

    for (;
         tmp_it != eppstein_res.begin() && tmp_it2 != lazy_eppstein_res.begin();
         --tmp_it, --tmp_it2)
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


    std::cout << "Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;
    std::cout << "Lazy eppstein: " << crono2.wallTime() / 1000
              << " milliseconds" << std::endl;
  }

  TEST(ButcherTest, final_network_test)
  {
    std::size_t       num_devices = 3;
    std::size_t const k           = 1000;

    std::vector<type_collection_weights> weight_maps;
    auto                                 model_butcher = real_butcher();

    auto const &model   = std::get<1>(model_butcher);
    auto       &butcher = std::get<0>(model_butcher);

    auto &graph = butcher.get_graph_ref();
    auto const &nodes = graph.get_nodes();

    basic_weight(graph, true);

    auto transmission_fun =
      basic_transmission(k, butcher.get_graph().get_nodes().size());

    Chrono crono;
    crono.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_paths_lazy_eppstein_linear(transmission_fun,
                                                            k);
    crono.stop();

    memory_type const gb  = 1024 * 1024 * 1024;
    memory_type const gb1 = gb;
    memory_type const gb4 = 4 * gb;
    memory_type const gb8 = 8 * gb;

    butcher.partition_memory_checker(lazy_eppstein_res[10].second,
                                     {gb1, gb4, gb8});

    std::cout << "Lazy Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
              << std::endl;

    auto const model_device =
      IO_Manager::reconstruct_model(lazy_eppstein_res.back().second,
                                    model,
                                    graph,
                                    std::get<2>(model_butcher));

    for (std::size_t i = 0; i < model_device.size(); ++i)
      IO_Manager::export_to_onnx(model_device[i].first,
                                 "version-RFB-640-inferred-" +
                                   std::to_string(i) + "-device-" +
                                   std::to_string(model_device[i].second) +
                                   ".onnx");
  }

  Butcher<GraphType>
  basic_butcher()
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(Content_type({}, {{"X0", 0}}));
    nodes.emplace_back(Content_type({{"X0", 0}}, {{"X1", 1}}));
    nodes.emplace_back(Content_type({{"X1", 1}}, {{"X2", 2}}));
    nodes.emplace_back(Content_type({{"X1", 1}}, {{"X3", 3}}));
    nodes.emplace_back(Content_type({{"X3", 3}}, {{"X4", 4}}));
    nodes.emplace_back(Content_type({{"X2", 2}, {"X4", 4}}, {{"X5", 5}}));
    nodes.emplace_back(Content_type({{"X5", 5}}, {{"X6", 6}}));
    nodes.emplace_back(Content_type({{"X6", 6}}, {{"X7", 7}}));

    GraphType graph(3, std::move(nodes));

    for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
      {
        graph.set_weigth(k, {0, 1}, 1000. / std::pow(2, k));
        graph.set_weigth(k, {1, 2}, 1000. / std::pow(2, k));
        graph.set_weigth(k, {1, 3}, 500. / std::pow(2, k));
        graph.set_weigth(k, {3, 4}, 500. / std::pow(2, k));
        graph.set_weigth(k, {2, 5}, 1000. / std::pow(2, k));
        graph.set_weigth(k, {4, 5}, 1000. / std::pow(2, k));
        graph.set_weigth(k, {5, 6}, 1000. / std::pow(2, k));
        graph.set_weigth(k, {6, 7}, 0.);
      }

    return Butcher(std::move(graph));
  }


  std::tuple<Butcher<graph_type>,
             onnx::ModelProto,
             std::map<node_id_type, node_id_type>>
  real_butcher()
  {
    std::string const path =
      "version-RFB-640-inferred.onnx"; //"version-RFB-640.onnx";
    auto tuple = IO_Manager::import_from_onnx(path, true, 3);

    return {Butcher(std::get<0>(tuple)),
            std::get<1>(tuple),
            std::get<2>(tuple)};
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