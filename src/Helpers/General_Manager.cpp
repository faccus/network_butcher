//
// Created by faccus on 7/12/22.
//
#include "../../include/Helpers/General_Manager.h"

std::function<weight_type(const node_id_type &, size_t, size_t)>
General_Manager::generate_bandwidth_transmission_function(
  const Parameters          &params,
  const Butcher<graph_type> &butcher)
{
  Computer_memory cm;
  auto const      mbps = 1000. / 8;

  std::function<weight_type(node_id_type const &, std::size_t, std::size_t)>
    transmission_weights = [&params,
                            &cm,
                            &butcher,
                            mbps](node_id_type const &node_id,
                                  std::size_t         first_device,
                                  std::size_t         second_device) {
      if (first_device == second_device)
        {
          return .0;
        }
      else
        {
          auto const mem = cm.compute_memory_usage_output(
            butcher.get_graph().get_nodes()[node_id]);
          auto const it = params.bandwidth.find({first_device, second_device});

          return mem / (it->second * mbps);
        }
    };
  return transmission_weights;
}

void
General_Manager::boot(std::string const &path)
{
  Parameters params = IO_Manager::read_parameters(path);
  auto [graph, model, link_id_nodeproto] =
    IO_Manager::import_from_onnx(params.model_path,
                                 true,
                                 params.devices.size());

  for (auto const &device : params.devices)
    {
      IO_Manager::import_weights_from_csv(graph,
                                          device.id,
                                          device.weights_path);
    }

  Butcher             butcher(std::move(graph));
  Weighted_Real_Paths paths;

  std::function<weight_type(const node_id_type &, size_t, size_t)>
    transmission_weights =
      General_Manager::generate_bandwidth_transmission_function(params,
                                                                butcher);

  switch (params.method)
    {
      case KSP_Method::Eppstein:
        paths = butcher.compute_k_shortest_paths_eppstein_linear(
          transmission_weights, params.K, params.backward_connections_allowed);
        break;
      case KSP_Method::Lazy_Eppstein:
        paths = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
          transmission_weights, params.K, params.backward_connections_allowed);
        break;
    }

  IO_Manager::export_network_partitions(
    params, graph, model, link_id_nodeproto, paths);
}