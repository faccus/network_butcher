#include "Butcher.h"


template <>
void
Butcher<graph_type>::block_graph_weights(
  Butcher<graph_type>::new_network                                                 &new_graph,
  const network_butcher_parameters::Parameters                                     &params,
  const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &transmission_weights) const
{
  network_butcher_io::Weight_importer_helpers::import_weights_aMLLibrary_local_block(new_graph, graph, params);

  auto const &nodes = new_graph.get_nodes();
  std::for_each(nodes.cbegin(),
                nodes.cend(),
                [&new_graph, &graph = graph, &transmission_weights](new_network::Node_Type const &node) {
                  auto const first = node.get_id();

                  for (auto const &second : new_graph.get_neighbors()[node.get_id()].second)
                    {
                      auto const &out_node = new_graph[second];

                      edge_type const edge = {first, second};

                      // The device id of the input node (=0 starting device, >0 other
                      // device)
                      auto const in_device_id = node.content.first;
                      // The device id of the output node (=0 starting device, >0 other
                      // device)
                      auto const out_device_id = out_node.content.first;

                      // Look for the nodes of the original graph that are
                      // represented by the output node (in the linearized
                      // graph)
                      auto const &outputs = *out_node.content.second;

                      // Look for the nodes of the original graph that are
                      // represented by the input node (in the linearized
                      // graph)
                      auto const &inputs = *node.content.second;

                      weight_type transmission_costs = .0;

                      // If we are dealing with the same device, the imported weight is OK
                      if (in_device_id == out_device_id)
                        {
                          continue;
                        }
                      else if (inputs.size() == 1)
                        {
                          // We just have to send the input from in_device to out_device
                          transmission_costs = transmission_weights(*inputs.cbegin(), in_device_id, out_device_id);
                        }
                      else if (outputs.size() == 1)
                        {
                          auto const &output       = *outputs.begin();
                          auto const &dependencies = graph.get_neighbors();

                          // The inputs on the original graph of the output node have to
                          // transmit their values to the output node
                          for (auto const &input : dependencies[output].first)
                            {
                              transmission_costs += transmission_weights(input, in_device_id, out_device_id);
                            }
                        }
                      else
                        {
                          std::cout << "Warning: we couldn't determine a weight!" << std::endl;
                        }

                      weight_type final_cost = transmission_costs + new_graph.get_weight(edge);
                      new_graph.set_weight(edge, final_cost);
                    }
                });
}


template <>
network_butcher_types::Weighted_Real_Paths
Butcher<graph_type>::compute_k_shortest_path(
  const std::function<weight_type(const node_id_type &, std::size_t, std::size_t)> &transmission_weights,
  const network_butcher_parameters::Parameters                                     &params) const
{
  auto [new_graph, connection_map] =
    block_graph(params.backward_connections_allowed, params.starting_device_id, params.ending_device_id);

  if (params.memory_constraint != network_butcher_parameters::Memory_Constraint_Type::None &&
      !params.backward_connections_allowed)
    remove_unfeasible_paths(params.devices, new_graph, params.memory_constraint_type);

  if (params.weight_import_mode == network_butcher_parameters::Weight_Import_Mode::aMLLibrary_inference_block)
    {
      block_graph_weights(new_graph, params, transmission_weights);
    }
  else
    {
      block_graph_weights(new_graph, transmission_weights);
    }


  std::unique_ptr<network_butcher_kfinder::KFinder<new_network>> kFinder;
  switch (params.method)
    {
      case network_butcher_parameters::KSP_Method::Eppstein:
        kFinder = std::make_unique<network_butcher_kfinder::KFinder_Eppstein<new_network>>(new_graph);
        break;
      case network_butcher_parameters::KSP_Method::Lazy_Eppstein:
        kFinder = std::make_unique<network_butcher_kfinder::KFinder_Lazy_Eppstein<new_network>>(new_graph);
        break;
      default:
        return {};
    }

  auto const res = kFinder->compute(params.K);
  return get_weighted_network_slice(res, new_graph);
}