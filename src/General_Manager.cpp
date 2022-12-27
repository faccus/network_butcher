//
// Created by faccus on 7/12/22.
//
#include "../include/General_Manager.h"

std::function<weight_type(const node_id_type &, size_t, size_t)>
network_butcher_io::General_Manager::Helper_Functions::generate_bandwidth_transmission_function(const network_butcher_parameters::Parameters &params,
                                                                              const graph_type &graph)
{
  // Conversion from bytes to mb
  auto const mbps = 1000. / 8;

  std::function<weight_type(node_id_type const &, std::size_t, std::size_t)> transmission_weights =
    [&params, &graph, mbps](node_id_type const &node_id, std::size_t first_device, std::size_t second_device) {
      auto const it = params.bandwidth.find({first_device, second_device});

      // If the bandwidth cannot be found, return 0
      if (it == params.bandwidth.cend())
        {
          return .0;
        }
      else
        {
          // The memory dimension of the output tensor for the given node
          auto const mem = network_butcher_computer::Computer_memory::compute_memory_usage_output(graph[node_id]);

          // Bandwidth
          auto const bdw = it->second.first;

          // Access delay
          auto const acc = it->second.second;

          return mem / (bdw * mbps) + acc;
        }
    };
  return transmission_weights;
}


void
network_butcher_io::General_Manager::Helper_Functions::import_weights(graph_type &graph, const network_butcher_parameters::Parameters &params)
{
  // Based on the weight_import_mode, a specific weight import function will be called for each device
  switch (params.weight_import_mode)
    {
        case network_butcher_parameters::Weight_Import_Mode::operation_time:
        case network_butcher_parameters::Weight_Import_Mode::aMLLibrary: {
          for (auto const &device : params.devices)
            {
              IO_Manager::import_weights(params.weight_import_mode, graph, device.weights_path, device.id);
            }

          break;
        }
        case network_butcher_parameters::Weight_Import_Mode::official_operation_time:
        case network_butcher_parameters::Weight_Import_Mode::multi_operation_time: {
          std::vector<std::size_t> devices;
          for (auto const &device : params.devices)
            devices.push_back(device.id);

          IO_Manager::import_weights(params.weight_import_mode, graph, params.devices.front().weights_path, devices);

          break;
        }
    }
}


void
network_butcher_io::General_Manager::boot(std::string const &path, bool performance)
{
  network_butcher_parameters::Parameters params = IO_Manager::read_parameters(path);
  boot(params, performance);
}

void
network_butcher_io::General_Manager::boot(const network_butcher_parameters::Parameters &params, bool performance)
{
  Chrono crono;
  crono.start();

  // Import the onnx model and populate the graph
  auto [graph, model, link_graph_model] =
    IO_Manager::import_from_onnx(params.model_path, true, true, params.devices.size());
  crono.stop();

  double const import_time = crono.wallTime();
  if (performance)
    std::cout << "Network import: " << import_time / 1000. << " ms" << std::endl;

  crono.start();

  // Import the weights
  Helper_Functions::import_weights(graph, params);
  crono.stop();

  double const import_weights_time = crono.wallTime();
  if (performance)
    std::cout << "Weight import: " << import_weights_time / 1000. << " ms" << std::endl;

  // Prepare the butcher...
  Butcher butcher(std::move(graph));

  crono.start();

  // Start the butchering... (compute the k shortest paths)
  auto const paths = butcher.compute_k_shortest_path(
    Helper_Functions::generate_bandwidth_transmission_function(params, butcher.get_graph()), params);
  crono.stop();

  double const butcher_time = crono.wallTime();
  if (performance)
    std::cout << "Butchering: " << butcher_time / 1000. << " ms" << std::endl;

  crono.start();
  // Export the butchered networks... (export the different partitions)
  IO_Manager::export_network_partitions(params, model, link_graph_model, paths);
  crono.stop();

  if (performance)
    {
      double const export_time = crono.wallTime();
      std::cout << "Export: " << export_time / 1000. << " ms" << std::endl;
      
      std::ofstream out_file;
      out_file.open(params.export_directory + "/report.txt");

      out_file << "Network import: " << import_time / 1000. << " ms" << std::endl;
      out_file << "Weight import: " << import_weights_time / 1000. << " ms" << std::endl;
      out_file << "Butchering: " << butcher_time / 1000. << " ms" << std::endl;
      out_file << "Export: " << export_time / 1000. << " ms" << std::endl;

      out_file << std::endl;

      out_file << "Number of found paths: " << paths.size() << std::endl;

      out_file.close();
    }
}
