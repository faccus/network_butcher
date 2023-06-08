#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "butcher.h"
#include "chrono.h"
#include "io_manager.h"
#include "test_class.h"

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;
  using namespace network_butcher::computer;
  using namespace network_butcher::parameters;

  using type_weight             = double;
  using type_collection_weights = std::map<std::pair<Node_Id_Type, Node_Id_Type>, type_weight>;

  using basic_type   = int;
  using Input        = Test_Class<int>;
  using Content_type = Content<Input>;
  using Node_type    = CNode<Content_type>;
  using Graph_type      = MWGraph<false, Node_type>;

  auto
  base_parameters(std::size_t k, bool backward, std::size_t num_devices) -> Parameters;

  auto
  eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices) -> Parameters;

  auto
  lazy_eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices) -> Parameters;

  auto basic_transmission(std::size_t, std::size_t)
    -> std::function<type_weight(Edge_Type const &, std::size_t, std::size_t)>;

  struct path_comparison
  {
    auto
    operator()(Weighted_Real_Path const &rhs, Weighted_Real_Path const &lhs) const -> bool;
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
      for (auto const &head : graph.get_output_nodes(tail))
        {
          auto const tmp_weight = node_weights_generator(random_engine);
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
            {
              graph.set_weight(k, {tail, head}, tmp_weight / std::pow(2, k));
            }
        }
  }


  TEST(ButcherTest, compute_k_shortest_paths_test_network_basic_weights)
  {
    std::string path        = "test_data/models/resnet18-v2-7-inferred";
    std::string extension   = ".onnx";
    std::size_t num_devices = 3;

    Butcher butcher(std::get<0>(io::IO_Manager::import_from_onnx(path + extension, true, true, num_devices)));

    auto       &graph = butcher.get_graph_ref();
    auto const &nodes = graph.get_nodes();
    auto const  k     = 1000;

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

    std::set<Weighted_Real_Path, path_comparison> eppstein;
    eppstein.insert(eppstein_res.begin(), eppstein_res.end());

    std::set<Weighted_Real_Path, path_comparison> lazy_eppstein;
    lazy_eppstein.insert(lazy_eppstein_res.begin(), lazy_eppstein_res.end());

    ASSERT_EQ(eppstein, lazy_eppstein);
  }


  auto
  basic_transmission(std::size_t devices, std::size_t size)
    -> std::function<type_weight(Edge_Type const &, std::size_t, std::size_t)>
  {
    return [devices, size](Edge_Type const &in_edge, std::size_t first, std::size_t second) {
      auto const &[input, tmp] = in_edge;
      if (input < size && first < 3 && second < 3)
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

  auto
  base_parameters(std::size_t k, bool backward, std::size_t num_devices) -> Parameters
  {
    Parameters res;
    res.ksp_params.K                      = k;
    res.devices                           = std::vector<Device>(num_devices);
    res.weights_params.weight_import_mode = parameters::Weight_Import_Mode::single_direct_read;
    res.block_graph_generation_params.use_bandwidth_to_manage_connections = false;

    for (std::size_t i = 0; i < res.devices.size(); ++i)
      res.devices[i].id = i;

    if (backward)
      {
        using g_type = Parameters::Weights::connection_type::element_type;
        g_type::Dependencies_Type deps(num_devices);

        for (std::size_t i = 0; i < num_devices; ++i)
          {
            for (std::size_t j = i; j < num_devices; ++j)
              {
                deps[i].second.insert(j);
                deps[j].first.insert(i);
              }
          }

        res.block_graph_generation_params.use_bandwidth_to_manage_connections = true;
        res.weights_params.bandwidth =
          std::make_unique<g_type>(g_type::Node_Collection_Type(num_devices), std::move(deps));

        for (std::size_t i = 0; i < num_devices; ++i)
          {
            for (std::size_t j = i + 1; j < num_devices; ++j)
              {
                res.weights_params.bandwidth->set_weight(std::make_pair(i, j), std::make_pair(1., 0.));
              }

            for (std::size_t j = 0; j < i; ++j)
              {
                res.weights_params.in_bandwidth[std::make_pair(i, j)]  = std::make_pair(1., 0.);
                res.weights_params.out_bandwidth[std::make_pair(i, j)] = std::make_pair(1., 0.);
              }
          }
      }

    res.block_graph_generation_params.memory_constraint  = false;
    res.block_graph_generation_params.starting_device_id = 0;
    res.block_graph_generation_params.ending_device_id   = 0;
    res.block_graph_generation_params.block_graph_mode   = parameters::Block_Graph_Generation_Mode::classic;

    return res;
  }

  auto
  eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices) -> Parameters
  {
    auto res              = base_parameters(k, backward, num_devices);
    res.ksp_params.method = KSP_Method::Eppstein;

    return res;
  }

  auto
  lazy_eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices) -> Parameters
  {
    auto res              = base_parameters(k, backward, num_devices);
    res.ksp_params.method = KSP_Method::Lazy_Eppstein;

    return res;
  }

  auto
  path_comparison::operator()(const network_butcher::types::Weighted_Real_Path &rhs,
                              const network_butcher::types::Weighted_Real_Path &lhs) const -> bool
  {
    return rhs.first < lhs.first || rhs.first == lhs.first && rhs.second < lhs.second;
  }
}; // namespace