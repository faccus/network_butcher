#include <iostream>
#include <random>

#include "GetPot"
#include "chrono.h"

#include "network_butcher.h"

/*
 * This main file generates a simple linear graph with 4 devices. The graph is linear since
 * the linearization phase of the algorithm doesn't influence the performances of the construction.
 * Weights are assigned randomly. We test how Constrained_Block_Graph_Builder performs on graphs with an
 * increasing number of nodes.
 * */

using namespace network_butcher;
using namespace network_butcher::types;

using Node_type = Node;
using GraphType = MWGraph<false, Node_type, long>;


template <class Graph>
void
basic_weight(Graph &graph, bool fully_random = false)
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

  std::uniform_int_distribution node_weights_generator{1, 100};

  for (std::size_t tail = 0; tail < graph.get_nodes().size(); ++tail)
    for (auto const &head : graph.get_output_nodes(tail))
      {
        auto const tmp_weight = node_weights_generator(random_engine) * 8;
        for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
          {
            graph.set_weight(k, std::make_pair(tail, head), tmp_weight / std::pow(2, k));
          }
      }
}

// Sample transmission: returns bandwidth + access time
std::function<GraphType::Weight_Type(Edge_Type const &, std::size_t, std::size_t)>
basic_transmission(parameters::Parameters::Weights const &weight_params, std::size_t num_nodes)
{
  return [&weight_params, num_nodes](Edge_Type const &in_edge, std::size_t first, std::size_t second) {
    auto const pair_device = std::make_pair(first, second);

    auto const &[tail, head] = in_edge;
    if (tail == 0 && weight_params.in_bandwidth.find(pair_device) != weight_params.in_bandwidth.cend())
      {
        auto const it = weight_params.in_bandwidth.find(pair_device);

        return it->second.first + it->second.second;
      }
    else if (head == num_nodes - 1 &&
             weight_params.out_bandwidth.find(pair_device) != weight_params.out_bandwidth.cend())
      {
        auto const it = weight_params.out_bandwidth.find(pair_device);

        return it->second.first + it->second.second;
      }
    else
      {
        if (weight_params.bandwidth->get_output_nodes(first).contains(second))
          {
            auto pair = weight_params.bandwidth->get_weight(pair_device);
            return pair.first + pair.second;
          }
        else
          {
            throw std::runtime_error("Missing device!");
          }
      }
  };
}

GraphType
basic_graph(std::size_t num_nodes)
{
  GraphType::Dependencies_Type deps(num_nodes);

  deps[0].second.insert(1);
  deps[1].first.insert(0);

  for (std::size_t i = 1; i < deps.size() - 1; ++i)
    {
      deps[i].second.insert(i + 1);
      deps[i + 1].first.insert(i);
    }

  return GraphType(4, std::vector<Node_type>(num_nodes), std::move(deps));
};

parameters::Parameters
generate_parameters()
{
  using g_type = parameters::Parameters::Weights::connection_type::element_type;
  parameters::Parameters res;

  res.ksp_params.K      = 5;
  res.ksp_params.method = parameters::KSP_Method::Lazy_Eppstein;
  res.devices           = std::vector<parameters::Device>(4);

  for (std::size_t i = 0; i < res.devices.size(); ++i)
    res.devices[i].id = i;

  res.block_graph_generation_params.memory_constraint = false;
  res.block_graph_generation_params.block_graph_mode  = parameters::Block_Graph_Generation_Mode::classic;
  res.block_graph_generation_params.use_bandwidth_to_manage_connections = true;

  g_type::Dependencies_Type deps(4);
  deps[0] = std::make_pair(std::set<Node_Id_Type>{0}, std::set<Node_Id_Type>{0, 1, 3});
  deps[1] = std::make_pair(std::set<Node_Id_Type>{0, 1, 2}, std::set<Node_Id_Type>{1, 2});
  deps[2] = std::make_pair(std::set<Node_Id_Type>{1, 2}, std::set<Node_Id_Type>{1, 2, 3});
  deps[3] = std::make_pair(std::set<Node_Id_Type>{0, 2, 3}, std::set<Node_Id_Type>{3});

  res.weights_params.bandwidth = std::make_unique<g_type>(g_type::Node_Collection_Type(4), std::move(deps));

  res.weights_params.bandwidth->set_weight(std::make_pair(0, 1), std::make_pair(10, 5));
  res.weights_params.bandwidth->set_weight(std::make_pair(0, 3), std::make_pair(80, 0));

  res.weights_params.bandwidth->set_weight(std::make_pair(1, 2), std::make_pair(20, 5));

  res.weights_params.bandwidth->set_weight(std::make_pair(2, 1), std::make_pair(10, 5));
  res.weights_params.bandwidth->set_weight(std::make_pair(2, 3), std::make_pair(20, 5));

  for (std::size_t i = 0; i < res.weights_params.bandwidth->size(); ++i)
    {
      res.weights_params.bandwidth->set_weight(std::make_pair(i, i),
                                               std::make_pair(std::numeric_limits<Bandwidth_Value_Type>::infinity(),
                                                              0.));
    }


  res.block_graph_generation_params.starting_device_id = 0;
  res.block_graph_generation_params.ending_device_id   = res.weights_params.bandwidth->size() - 1;

  return res;
}

std::size_t
simple_pow(std::size_t base, std::size_t exp)
{
  std::size_t res = 1;
  for (; exp > 0; --exp)
    {
      res *= base;
    }

  return res;
}

int
main(int argc, char **argv)
{
  GetPot      command_line(argc, argv);
  std::string export_path = "report_Constrained_Builder.txt";

  std::vector<std::tuple<std::string, Time_Type>> results;
  std::size_t                                     num_tests = command_line("num_tests", 10);
  std::size_t                                     max_power = command_line("max_power", 20);

  Chrono crono;
  {
    std::ofstream out_file(export_path);
    out_file << "NumNodes,TotalTime" << std::endl;
    out_file.close();
  }

  for (std::size_t power = 10; power <= max_power; ++power)
    {
      std::size_t nodes = simple_pow(2, power);
      Time_Type   time  = 0.;

      auto params       = generate_parameters();
      auto graph        = basic_graph(nodes);
      auto transmission = basic_transmission(params.weights_params, nodes);

      basic_weight(graph);


      for (std::size_t test_num = 0; test_num < num_tests; ++test_num)
        {
          Constrained_Block_Graph_Builder<GraphType> builder(graph, params);
          builder.construct_weights(transmission);

          crono.start();
          builder.construct_block_graph();
          crono.stop();

          Time_Type local_time = crono.wallTime();
          time += local_time;

          std::cout << "Test #" << Utilities::custom_to_string(test_num + 1) << ": " << local_time / 1000. << " ms"
                    << std::endl;
        }

      time /= (num_tests * static_cast<long double>(1000.));

      std::cout << std::endl
                << "Total time average for " << nodes << " nodes: " << time << " ms" << std::endl
                << std::endl;

      std::ofstream out_file(export_path, std::ios_base::app);
      out_file << nodes << "," << time << std::endl;
      out_file.close();
    }
}