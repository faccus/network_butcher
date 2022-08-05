//
// Created by faccus on 7/12/22.
//
#include "../include/General_Manager.h"

std::function<weight_type(const node_id_type &, size_t, size_t)>
network_butcher_io::General_Manager::generate_bandwidth_transmission_function(const Parameters &params,
                                                                              const graph_type &graph)
{
  auto const mbps = 1000. / 8;

  std::function<weight_type(node_id_type const &, std::size_t, std::size_t)> transmission_weights =
    [&params, &graph, mbps](node_id_type const &node_id, std::size_t first_device, std::size_t second_device) {
      if (first_device >= second_device)
        {
          return .0;
        }
      else
        {
          auto const mem = network_butcher_computer::Computer_memory::compute_memory_usage_output(graph[node_id]);
          auto const it  = params.bandwidth.find({first_device, second_device});

          return mem / (it->second * mbps);
        }
    };
  return transmission_weights;
}


void
network_butcher_io::General_Manager::import_weights(graph_type &graph, const Parameters &params)
{
  switch (params.weight_import_mode)
    {
      case Weight_Import_Mode::operation_time:
      case Weight_Import_Mode::aMLLibrary:
        for (auto const &device : params.devices)
          {
            IO_Manager::import_weights(params.weight_import_mode, graph, device.weights_path, device.id);
          }

        break;
        case Weight_Import_Mode::multi_operation_time: {
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
  Parameters params = IO_Manager::read_parameters(path);
  boot(params, performance);
}

void
network_butcher_io::General_Manager::boot(const Parameters &params, bool performance)
{
  Chrono crono;
  crono.start();

  auto [graph, model, link_graph_model] = IO_Manager::import_from_onnx(params.model_path, true, params.devices.size());
  crono.stop();

  if (performance)
    std::cout << "Network import: " << crono.wallTime() << " ns" << std::endl;

  crono.start();
  import_weights(graph, params);
  crono.stop();

  if (performance)
    std::cout << "Weight import: " << crono.wallTime() << " ns" << std::endl;

  Butcher butcher(std::move(graph));

  crono.start();
  auto const paths = butcher.compute_k_shortest_path(
    General_Manager::generate_bandwidth_transmission_function(params, butcher.get_graph()), params);
  crono.stop();

  if (performance)
    std::cout << "Butchering: " << crono.wallTime() << " ns" << std::endl;

  crono.start();
  IO_Manager::export_network_partitions(params, graph, model, link_graph_model, paths);
  crono.stop();

  if (performance)
    std::cout << "Export: " << crono.wallTime() << " ns" << std::endl;
}