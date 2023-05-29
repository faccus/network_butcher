//
// Created by faccus on 12/12/21.
//

#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "Butcher.h"
#include "IO_Manager.h"
#include "TestClass.h"
#include "chrono.h"

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;
  using namespace network_butcher::computer;
  using namespace network_butcher::parameters;

  using type_weight             = double;
  using type_collection_weights = std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  using basic_type   = int;
  using Input        = TestMemoryUsage<int>;
  using Content_type = Content<Input>;
  using Node_type    = CNode<Content_type>;

  using Graph_type      = MWGraph<false, Node_type>;
  using Real_Graph_Type = MWGraph<false, graph_input_type>;


  Butcher<Graph_type>
  basic_butcher(int);

  std::tuple<Butcher<graph_type>, onnx::ModelProto, std::map<node_id_type, node_id_type>>
  real_butcher();

  parameters::Parameters
  base_parameters(std::size_t k, bool backward, std::size_t num_devices);

  parameters::Parameters
  eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices);

  parameters::Parameters
  lazy_eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices);
  parameters::Parameters
  real_parameters(std::size_t k, bool backward);

  struct path_comparison
  {
    bool
    operator()(Weighted_Real_Path const &rhs, Weighted_Real_Path const &lhs) const;
  };

  template <class Graph>
  void
  complete_weights(Graph &graph)
  {
    auto const num_nodes = graph.get_nodes().size();

    for (node_id_type tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : graph.get_output_nodes(tail))
        {
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
            {
              if (graph.get_weight(k, {tail, head}) == -1.)
                graph.set_weight(k, {tail, head}, 0.);
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
      for (auto const &head : graph.get_output_nodes(tail))
        {
          auto const tmp_weight = node_weights_generator(random_engine);
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
            {
              graph.set_weight(k, {tail, head}, tmp_weight / std::pow(2, k));
            }
        }
  }


  std::function<type_weight(edge_type const &, std::size_t, std::size_t)> basic_transmission(std::size_t, std::size_t);


  void
  real_weight(Real_Graph_Type &);

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  real_transmission(Real_Graph_Type const &);


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


  TEST(ButcherTest, compute_k_shortest_paths_eppstein_vs_lazy_deterministic_multiple)
  {
    std::size_t       num_devices = 3;
    std::size_t const num_nodes   = 1000;

    std::size_t k = 1000;

    std::size_t number_of_tests = 10;


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

        std::set<Weighted_Real_Path, path_comparison> eppstein;
        eppstein.insert(eppstein_res.begin(), eppstein_res.end());

        std::set<Weighted_Real_Path, path_comparison> lazy_eppstein;
        lazy_eppstein.insert(lazy_eppstein_res.begin(), lazy_eppstein_res.end());

        ASSERT_EQ(eppstein, lazy_eppstein);

        std::cout << "Test number #" << (num_test + 1) << ", Lazy: " << time_instance_lazy / 1000
                  << " ms, Epp: " << time_instance_std / 1000 << " ms"
                  << "Average Lazy: " << time_lazy / ((num_test + 1) * 1000) << ", Found path: " << lazy_eppstein.size()
                  << std::endl;
      }

    std::cout << "Lazy Eppstein: " << time_lazy / 1000 / number_of_tests << " milliseconds" << std::endl;

    std::cout << "Eppstein: " << time_std / 1000 / number_of_tests << " milliseconds" << std::endl;
  }


  Butcher<Graph_type>
  basic_butcher(int num_nodes)
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(std::move(Content_Builder<Input>().set_output({{"X0", 0}})).build());
    for (int n = 1; n < num_nodes - 1; ++n)
      nodes.emplace_back(std::move(Content_Builder<Input>()
                                     .set_input({{"X" + std::to_string(n - 1), n - 1}})
                                     .set_output({{"X" + std::to_string(n), n}}))
                           .build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X" + std::to_string(num_nodes - 2), num_nodes - 2}})).build());

    Graph_type graph_cons(3, std::move(nodes));

    return Butcher(std::move(graph_cons));
  }

  std::function<type_weight(edge_type const &, std::size_t, std::size_t)>
  basic_transmission(std::size_t devices, std::size_t size)
  {
    return [devices, size](edge_type const &in_edge, std::size_t first, std::size_t second) {
      auto const &[input, tmp] = in_edge;
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

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  real_transmission(Real_Graph_Type const &graph)
  {
    auto const mbps = 1000. / 8;

    return [&graph, mbps](node_id_type const &node, std::size_t from_device, std::size_t to_device) {
      auto const mem_to_transmit = Computer_memory::compute_memory_usage_output(graph.get_nodes()[node]);

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
    std::string const path  = "test_data/models/version-RFB-640-inferred.onnx"; //"version-RFB-640.onnx";
    auto              tuple = io::IO_Manager::import_from_onnx(path, true, true, 3);
    auto             &graph = std::get<0>(tuple);

    Parameters params;
    params.weights_params.weight_import_mode = Weight_Import_Mode::multiple_direct_read;

    params.devices.push_back(Device{0, "", 100, "test_data/weights/aMLLibrary_prediction_pi.csv", "pred"});
    params.devices.push_back(Device{1, "", 100, "test_data/weights/aMLLibrary_prediction_tegra.csv", "pred"});
    params.devices.push_back(Device{2, "", 100, "test_data/weights/aMLLibrary_prediction_tegra.csv", "pred"});

    io::IO_Manager::import_weights(graph, params);

    complete_weights(graph);

    return {Butcher(graph), std::get<1>(tuple), std::get<2>(tuple)};
  }

  parameters::Parameters
  base_parameters(std::size_t k, bool backward, std::size_t num_devices)
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
        using g_type = parameters::Parameters::Weights::connection_type::element_type;
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


    res.block_graph_generation_params.memory_constraint_type = Memory_Constraint_Type::None;
    res.block_graph_generation_params.starting_device_id     = 0;
    res.block_graph_generation_params.ending_device_id       = 0;
    res.block_graph_generation_params.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;

    return res;
  }

  Parameters
  eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices)
  {
    auto res              = base_parameters(k, backward, num_devices);
    res.ksp_params.method = KSP_Method::Eppstein;

    return res;
  }

  Parameters
  lazy_eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices)
  {
    auto res              = base_parameters(k, backward, num_devices);
    res.ksp_params.method = KSP_Method::Lazy_Eppstein;

    return res;
  }

  bool
  path_comparison::operator()(const network_butcher::types::Weighted_Real_Path &rhs,
                              const network_butcher::types::Weighted_Real_Path &lhs) const
  {
    return rhs.first < lhs.first || rhs.first == lhs.first && rhs.second < lhs.second;
  }
}; // namespace