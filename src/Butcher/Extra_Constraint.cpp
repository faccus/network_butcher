//
// Created by faccus on 24/04/23.
//
#include "Extra_Constraint.h"

namespace network_butcher::constraints
{
  std::unique_ptr<Extra_Constraint>
  Extra_Constraint::copy() const
  {
    return {};
  }

  void
  Bandwidth_Constraint::apply_constraint(block_graph_type &graph) const
  {
    auto const num_devices = params.devices.size();

    // If there are two device or less, the constraint doesn't apply
    if (num_devices <= 1)
      return;

    auto       &deps      = graph.get_neighbors_ref();
    auto const  num_nodes = graph.size();
    auto const &bandwidth = params.weights_params.bandwidth;

    // Simple helper function used to remove the specified neighbors
    auto dep_deleter = [&deps](node_id_type input_node_id, node_id_type output_node_id) {
      deps[input_node_id].second.erase(output_node_id);
      deps[output_node_id].first.erase(input_node_id);
    };

    // Loop through all the devices
    for (auto const &i_dev : params.devices)
      {
        for (auto const &j_dev : params.devices)
          {
            // Check if the couple of devices can be connected
            if (params.backward_connections_allowed && i_dev.id != j_dev.id ||
                !params.backward_connections_allowed && i_dev.id < j_dev.id)
              {
                // Find their bandwidth
                auto band_it = bandwidth.find(std::make_pair(i_dev.id, j_dev.id));

                // If no bandwidth was specified, we should treat it as 0. Thus, no connection is allowed!
                if (band_it == bandwidth.cend() || band_it->second.first == 0.)
                  {
                    // If the starting node device is i_dev, then it cannot be connected to j_dev
                    if (graph.get_nodes().front().content.first == i_dev.id)
                      {
                        dep_deleter(0, j_dev.id + 1);
                      }

                    node_id_type counter = 1 + i_dev.id;
                    // Loop through the nodes in the block graph and remove the related neighbors
                    for (; counter < (num_nodes - num_devices - 1); counter += num_devices)
                      {
                        dep_deleter(counter, counter - i_dev.id + j_dev.id + num_devices);
                      }

                    // If the final node device is j_dev, then it cannot have an input connection to device i_dev
                    if (graph.get_nodes().back().content.first == j_dev.id)
                      {
                        dep_deleter(num_nodes - 1 - num_devices + i_dev.id, num_nodes - 1);
                      }
                  }
              }
          }
      }
  }

  std::unique_ptr<Extra_Constraint>
  Bandwidth_Constraint::copy() const
  {
    return std::make_unique<Bandwidth_Constraint>(*this);
  }
} // namespace network_butcher::constraints