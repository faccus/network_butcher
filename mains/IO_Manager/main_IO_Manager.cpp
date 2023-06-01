//
// Created by faccus on 30/05/23.
//

#include "Butcher.h"
#include "IO_Manager.h"
#include "TestClass.h"
#include "chrono.h"

#include "Starting_traits.h"
#include "Utilities.h"

#include "GetPot"

#include <fstream>
#include <random>

using namespace network_butcher;
using namespace network_butcher::types;
using namespace network_butcher::computer;
using namespace network_butcher::parameters;

using time_type = long double;

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

parameters::Parameters
base_parameters(std::size_t k, std::size_t num_devices)
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
  res.weights_params.bandwidth = std::make_unique<g_type>(g_type::Node_Collection_Type(num_devices), std::move(deps));

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

      res.weights_params.bandwidth->set_weight(std::make_pair(i, i),
                                               std::make_pair(std::numeric_limits<bandwidth_type>::infinity(), 0.));
    }


  res.block_graph_generation_params.memory_constraint_type = Memory_Constraint_Type::None;
  res.block_graph_generation_params.starting_device_id     = 0;
  res.block_graph_generation_params.ending_device_id       = 1;
  res.block_graph_generation_params.block_graph_mode       = parameters::Block_Graph_Generation_Mode::output;

  return res;
};

Parameters
lazy_eppstein_parameters(std::size_t k, std::size_t num_devices)
{
  auto res              = base_parameters(k, num_devices);
  res.ksp_params.method = KSP_Method::Lazy_Eppstein;

  return res;
};


std::function<weight_type(edge_type const &, std::size_t, std::size_t)>
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


std::vector<std::string>
get_test_names()
{
  return {"mobilenet_v2-inferred",
          "resnet18-v2-7-inferred",
          "version-RFB-640-inferred",
          "yolov5s_shape_inferred",
          "age_googlenet_shapes_only"};
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
  std::string export_path = "report_IO_Manager.txt";

  std::vector<std::tuple<std::string, time_type>> results;
  std::size_t                                     num_tests   = command_line("num_tests", 10);
  std::size_t                                     max_k_power = command_line("max_k_power", 11);
  std::size_t                                     num_devices = 3;

  #if PARALLEL_OPENMP
    std::cout << "Is OpenMP enabled? Let's check it!" << std::endl;

    int nthreads, tid;

  #  pragma omp parallel default(none) private(nthreads, tid)
    {
      /* Obtain thread number */
      tid = omp_get_thread_num();
      printf("Hello world from omp thread %d\n", tid);

      /* Only master thread does this */
      if (tid == 0)
        {
          nthreads = omp_get_num_threads();
          printf("Number of threads = %d\n", nthreads);
        }

    } /* All threads join master thread and disband */
  #endif

  Chrono crono;

  {
    std::ofstream out_file(export_path);
    out_file << "file,k,Export" << std::endl;
    out_file.close();
  }


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

      for (std::size_t k_power = 5; k_power < max_k_power; ++k_power)
        {
          std::size_t k = simple_pow(2, k_power);

          auto const params            = lazy_eppstein_parameters(k, num_devices);
          auto const lazy_eppstein_res = butcher.compute_k_shortest_path(transmission_fun, params);

          time_type time = 0.;

          std::cout << "K=" << k << std::endl;


          if (lazy_eppstein_res.size() < k)
            break;


          // The actual test starts here:
          for (std::size_t i = 0; i < num_tests; ++i)
            {
              Utilities::directory_delete(params.model_params.export_directory);

              crono.start();
              io::IO_Manager::export_network_partitions(params, model, map, lazy_eppstein_res);
              crono.stop();

              time_type local_time = crono.wallTime();
              time += local_time;

              std::cout << "Test #" << Utilities::custom_to_string(i + 1) << ": " << local_time / 1000. << " ms"
                        << std::endl;
            }
          time /= (num_tests * static_cast<long double>(1000.));
          std::cout << std::endl << "K: " << k << ", T: " << time << " ms" << std::endl << std::endl;


          std::ofstream out_file(export_path, std::ios_base::app);
          out_file << file_name << "," << k << "," << time << std::endl;
          out_file.close();
        }
    }
}