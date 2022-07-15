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

  Parameters
  eppstein_parameters(std::size_t k);

  Parameters
  lazy_eppstein_parameters(std::size_t k);

  template <class Graph>
  void
  complete_weights(Graph &graph)
  {
    auto const  num_nodes    = graph.get_nodes().size();
    auto const &dependencies = graph.get_dependencies();

    for (node_id_type tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : dependencies[tail].second) {
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k) {
              if (graph.get_weigth(k, {tail, head}) == -1.)
                graph.set_weigth(k, {tail, head}, 0.);
            }
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

    auto const res = butcher.compute_k_shortest_path(
      basic_transmission(graph.get_num_devices(), num_nodes),
      eppstein_parameters(1000));

    ASSERT_EQ(res.size(), 81);
  }

  TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_linear)
  {
    std::size_t num_devices = 3;

    auto butcher = basic_butcher();

    auto      &graph     = butcher.get_graph_ref();
    auto const num_nodes = graph.get_nodes().size();

    auto transmission_fun = basic_transmission(num_devices, num_nodes);

    auto const res = butcher.compute_k_shortest_path(
      basic_transmission(graph.get_num_devices(), num_nodes),
      lazy_eppstein_parameters(1000));

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
      butcher.compute_k_shortest_path(transmission_fun, eppstein_parameters(k));
    crono.stop();


    Chrono crono2;
    crono2.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_path(transmission_fun,
                                      lazy_eppstein_parameters(k));

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
    std::cout << "Lazy compute: " << crono2.wallTime() / 1000
              << " milliseconds" << std::endl;
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

  Parameters
  eppstein_parameters(std::size_t k) {
    Parameters res;
    res.K = k;
    res.backward_connections_allowed = true;
    res.method = KSP_Method::Eppstein;

    return res;
  }

  Parameters
  lazy_eppstein_parameters(std::size_t k) {
    Parameters res;
    res.K = k;
    res.backward_connections_allowed = true;
    res.method = KSP_Method::Lazy_Eppstein;

    return res;
  }
} // namespace butcher_test_namespace