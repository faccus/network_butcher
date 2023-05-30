//
// Created by faccus on 30/05/23.
//

#include "Butcher.h"
#include "IO_Manager.h"
#include "TestClass.h"
#include "chrono.h"

#include "GetPot"

#include <fstream>
#include <random>

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

parameters::Parameters
base_parameters(std::size_t k, bool backward, std::size_t num_devices);

parameters::Parameters
eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices);

parameters::Parameters
lazy_eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices);


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

struct path_comparison
{
  bool
  operator()(Weighted_Real_Path const &rhs, Weighted_Real_Path const &lhs) const
  {
    return rhs.first < lhs.first || rhs.first == lhs.first && rhs.second < lhs.second;
  };
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

  std::uniform_int_distribution node_weights_generator{1, 100};

  for (std::size_t tail = 0; tail < graph.get_nodes().size(); ++tail)
    for (auto const &head : graph.get_output_nodes(tail))
      {
        auto const tmp_weight = node_weights_generator(random_engine);
        for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
          {
            graph.set_weight(k, std::make_pair(tail, head), tmp_weight / std::pow(2, k));
          }
      }
}


std::function<type_weight(edge_type const &, std::size_t, std::size_t)>
basic_transmission(std::size_t devices, std::size_t size)
{
  return [devices, size](edge_type const &in_edge, std::size_t first, std::size_t second) {
    auto const &[input, tmp] = in_edge;
    if (input < size && first < devices && second < devices)
      {
        auto in_device_id  = first;
        auto out_device_id = second;

        if (in_device_id > out_device_id)
          std::swap(in_device_id, out_device_id);

        if (out_device_id - in_device_id == 2)
          return 80.;
        else if (out_device_id - in_device_id == 1)
          {
            if (out_device_id == 2)
              return 40.;
            else
              return 20.;
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
base_parameters(std::size_t k, bool backward, std::size_t num_devices)
{
  Parameters res;
  res.ksp_params.K                      = k;
  res.devices                           = std::vector<Device>(num_devices);
  res.weights_params.weight_import_mode = parameters::Weight_Import_Mode::single_direct_read;
  res.block_graph_generation_params.use_bandwidth_to_manage_connections = false;
  res.model_params.export_directory                                     = "report_IO_Manager";
  res.model_params.model_name                                           = "model";

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
  res.block_graph_generation_params.ending_device_id       = 1;
  res.block_graph_generation_params.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;

  return res;
}

Parameters
lazy_eppstein_parameters(std::size_t k, bool backward, std::size_t num_devices)
{
  auto res              = base_parameters(k, backward, num_devices);
  res.ksp_params.method = KSP_Method::Lazy_Eppstein;

  return res;
}


std::vector<std::string>
get_test_names()
{
  return {"mobilenet_v2-inferred", "resnet18-v2-7-inferred", "version-RFB-640-inferred", "yolov5s_shape_inferred"};
}

int
main(int argc, char **argv)
{
  GetPot command_line(argc, argv);

  std::vector<std::tuple<std::string, long double>> results;
  std::size_t                                       num_tests   = command_line("num_tests", 10);
  std::size_t                                       num_devices = 3, k = 1000;


  Chrono crono;
  for (auto const &file_name : get_test_names())
    {
      std::string input = "test_data/models/" + file_name + ".onnx";

      std::cout << "Processing file: " << input << std::endl;

      auto [tmp_graph, model, map] = io::IO_Manager::import_from_onnx(input, true, true, num_devices);
      Butcher butcher(std::move(tmp_graph));

      auto       &graph = butcher.get_graph_ref();
      auto const &nodes = graph.get_nodes();

      basic_weight(graph, false);
      auto const transmission_fun = basic_transmission(num_devices, nodes.size());

      auto const params            = lazy_eppstein_parameters(k, true, num_devices);
      auto const lazy_eppstein_res = butcher.compute_k_shortest_path(transmission_fun, params);

      long double time = .0;


      // The actual test starts here:
      for (std::size_t i = 0; i < num_tests; ++i)
        {
          Utilities::directory_delete(params.model_params.export_directory);

          crono.start();
          io::IO_Manager::export_network_partitions(params, model, map, lazy_eppstein_res);
          crono.stop();

          long double local_time = crono.wallTime();
          time += local_time;

          std::cout << "Test #" << Utilities::custom_to_string(i+1) << ": " << local_time / 1000. << " ms" << std::endl;
        }

      time /= (num_tests * static_cast<long double>(1000.));

      std::cout << std::endl << "Export time average " << time << " ms" << std::endl << std::endl;

      results.emplace_back(file_name, time);
    }

  std::string   export_path = "report_IO_Manager.txt";
  std::ofstream out_file(export_path);

  out_file << "Test,ExportTime" << std::endl;
  for (auto const &[name, time] : results)
    out_file << name << "," << time << std::endl;
}