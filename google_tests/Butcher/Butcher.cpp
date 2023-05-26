//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "Butcher.h"
#include "TestClass.h"
#include "chrono.h"

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::computer;
  using namespace network_butcher::types;

  using type_weight             = double;
  using type_collection_weights = std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  using basic_type   = int;
  using Input        = TestMemoryUsage<int>;
  using Content_type = Content<Input>;
  using Node_type    = CNode<Content_type>;
  using GraphType    = MWGraph<false, Node_type>;

  GraphType basic_graph(std::size_t);

  Butcher<GraphType>
  basic_butcher();

  parameters::Parameters
  basic_parameters(std::size_t k, std::size_t num_devices);

  parameters::Parameters
  eppstein_parameters(std::size_t k, std::size_t num_devices);

  parameters::Parameters
  lazy_eppstein_parameters(std::size_t k, std::size_t num_devices);

  struct path_comparison
  {
    bool
    operator()(Weighted_Real_Path const &rhs, Weighted_Real_Path const &lhs) const;
  };

  template <class Graph>
  void
  complete_weights(Graph &graph)
  {
    auto const  num_nodes    = graph.get_nodes().size();
    auto const &dependencies = graph.get_neighbors();

    for (node_id_type tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : dependencies[tail].second)
        {
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
            {
              if (graph.get_weight(k, {tail, head}) == -1.)
                graph.set_weight(k, {tail, head}, 0.);
            }
        }
  };


  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)> basic_transmission(std::size_t,
                                                                                                std::size_t);

  TEST(ButcherTest, compute_k_shortest_paths_eppstein_linear)
  {
    std::size_t num_devices = 3;

    auto butcher = basic_butcher();

    auto const &graph     = butcher.get_graph();
    auto const  num_nodes = graph.get_nodes().size();

    auto const res = butcher.compute_k_shortest_path(basic_transmission(graph.get_num_devices(), num_nodes),
                                                     eppstein_parameters(1000, num_devices));

    ASSERT_EQ(res.size(), 81);
  }

  TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_linear)
  {
    std::size_t num_devices = 3;

    auto butcher = basic_butcher();

    auto      &graph     = butcher.get_graph();
    auto const num_nodes = graph.get_nodes().size();

    auto transmission_fun = basic_transmission(num_devices, num_nodes);

    auto const res = butcher.compute_k_shortest_path(basic_transmission(graph.get_num_devices(), num_nodes),
                                                     lazy_eppstein_parameters(1000, num_devices));

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
    auto eppstein_res = butcher.compute_k_shortest_path(transmission_fun, eppstein_parameters(k, num_devices));
    crono.stop();


    Chrono crono2;
    crono2.start();
    auto lazy_eppstein_res =
      butcher.compute_k_shortest_path(transmission_fun, lazy_eppstein_parameters(k, num_devices));

    crono2.stop();
    crono2.wallTime();

    std::set<Weighted_Real_Path, path_comparison> eppstein;
    eppstein.insert(eppstein_res.begin(), eppstein_res.end());

    std::set<Weighted_Real_Path, path_comparison> lazy_eppstein;
    lazy_eppstein.insert(lazy_eppstein_res.begin(), lazy_eppstein_res.end());

    ASSERT_EQ(eppstein, lazy_eppstein);
  }


  GraphType
  basic_graph(std::size_t dev)
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(std::move(Content_Builder<Input>().set_output({{"X0", 0}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X0", 0}}).set_output({{"X1", 1}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X1", 1}}).set_output({{"X2", 2}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X1", 1}}).set_output({{"X3", 3}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X3", 3}}).set_output({{"X4", 4}})).build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X2", 2}, {"X4", 4}}).set_output({{"X5", 5}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X5", 5}}).set_output({{"X6", 6}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X6", 6}}).set_output({{"X7", 7}})).build());

    return GraphType(dev, std::move(nodes));
  }

  Butcher<GraphType>
  basic_butcher()
  {
    auto graph = basic_graph(3);

    for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
      {
        graph.set_weight(k, {0, 1}, 1000. / std::pow(2, k) + k);
        graph.set_weight(k, {1, 2}, 1000. / std::pow(2, k) + k);
        graph.set_weight(k, {1, 3}, 500. / std::pow(2, k) + k);
        graph.set_weight(k, {3, 4}, 500. / std::pow(2, k) + k);
        graph.set_weight(k, {2, 5}, 1000. / std::pow(2, k) + k);
        graph.set_weight(k, {4, 5}, 1000. / std::pow(2, k) + k);
        graph.set_weight(k, {5, 6}, 1000. / std::pow(2, k) + k);
        graph.set_weight(k, {6, 7}, 0.);
      }

    return Butcher(std::move(graph));
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

  parameters::Parameters
  basic_parameters(std::size_t k, std::size_t num_devices)
  {
    parameters::Parameters res;

    res.ksp_params.K                                               = k;
    res.block_graph_generation_params.backward_connections_allowed = true;
    res.devices                                                    = std::vector<parameters::Device>(num_devices);

    res.weights_params.weight_import_mode = parameters::Weight_Import_Mode::single_direct_read;

    for (std::size_t i = 0; i < res.devices.size(); ++i)
      res.devices[i].id = i;

    for (std::size_t i = 0; i < num_devices; ++i)
      {
        for (std::size_t j = 0; j < num_devices; ++j)
          {
            if (i != j)
              {
                res.weights_params.bandwidth[std::make_pair(i, j)] = std::make_pair(1., 0.);
              }
          }
      }


    res.block_graph_generation_params.memory_constraint_type = parameters::Memory_Constraint_Type::None;
    res.block_graph_generation_params.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;

    res.block_graph_generation_params.starting_device_id = 0;
    res.block_graph_generation_params.ending_device_id   = 0;

    return res;
  }

  parameters::Parameters
  eppstein_parameters(std::size_t k, std::size_t num_devices)
  {
    auto res              = basic_parameters(k, num_devices);
    res.ksp_params.method = parameters::KSP_Method::Eppstein;

    return res;
  }

  parameters::Parameters
  lazy_eppstein_parameters(std::size_t k, std::size_t num_devices)
  {
    auto res              = basic_parameters(k, num_devices);
    res.ksp_params.method = parameters::KSP_Method::Lazy_Eppstein;

    return res;
  }

  bool
  path_comparison::operator()(const network_butcher::types::Weighted_Real_Path &rhs,
                              const network_butcher::types::Weighted_Real_Path &lhs) const
  {
    return rhs.first < lhs.first || rhs.first == lhs.first && rhs.second < lhs.second;
  }
} // namespace