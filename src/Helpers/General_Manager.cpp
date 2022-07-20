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
General_Manager::import_weights(graph_type &graph, const Parameters &params)
{
  for (auto const &device : params.devices)
    {
      IO_Manager::import_weights_from_csv(graph,
                                          device.id,
                                          device.weights_path);
    }
}

void
General_Manager::boot(std::string const &path)
{
  Parameters params = IO_Manager::read_parameters(path);

  auto [graph, model, link_graph_model] =
    IO_Manager::import_from_onnx(params.model_path,
                                 true,
                                 params.devices.size());
  import_weights(graph, params);

  Butcher             butcher(std::move(graph));

  auto const paths = butcher.compute_k_shortest_path(
    General_Manager::generate_bandwidth_transmission_function(params, butcher),
    params);


  IO_Manager::export_network_partitions(
    params, graph, model, link_graph_model, paths);
}