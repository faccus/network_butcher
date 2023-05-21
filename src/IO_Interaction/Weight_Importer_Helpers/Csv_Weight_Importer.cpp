//
// Created by faccus on 13/04/23.
//
#include "Csv_Weight_Importer.h"

namespace network_butcher::io
{
  template <>
  void
  Csv_Weight_Importer<graph_type>::import_weights(
    std::function<bool(graph_type::Node_Type const &)> const &extra_condition)
  {
    // Are we dealing with a single .csv file?
    bool       single_call = check_paths();
    auto const data        = get_data(single_call);

    check_entries(data, single_call);

    // Import the weights
    for (std::size_t i = 0; i < devices.size(); ++i)
      {
        auto const &entry = single_call ? relevant_entries[i] : (relevant_entries[i] + "_" + std::to_string(i));

        auto const data_it = data.find(entry);

        auto const &values = data_it->second;
        auto        it     = ++graph.cbegin();

        for (auto const &weight : values)
          {
            while (extra_condition != nullptr && it != graph.cend() && !extra_condition(*it))
              ++it;

            if (it == graph.cend())
              break;

            for (auto const &in : graph.get_input_nodes(it->get_id()))
              {
                graph.set_weight(devices[i], {in, it->get_id()}, weight);
              }

            ++it;
          }
      }
  }

  template <>
  void
  Csv_Weight_Importer<block_graph_type>::import_weights(
    const std::function<bool(const block_graph_type::Node_Type &)> &extra_condition)
  {
    // Are we dealing with a single .csv file?
    bool       single_call = check_paths();
    auto const data        = get_data(single_call);

    check_entries(data, single_call);

    // Helper function used to skip to the next node of the same device
    auto const skip_device = [end = graph.cend()](auto &it, auto const &dev) {
      for (std::size_t j = 0; it != end && j < dev; ++j, ++it)
        ;
    };

    // Import the weights
    for (std::size_t i = 0; i < devices.size(); ++i)
      {
        auto const &device = devices[i];

        auto it = ++graph.cbegin();
        skip_device(it, device);

        auto const &weights = single_call ? data.find(relevant_entries[i])->second :
                                            data.find(relevant_entries[i] + "_" + std::to_string(i))->second;

        if ((!single_call || i == 0) && weights.size() != (graph.size() - 2) / devices.size())
          {
            std::cout << "The number of weights (" << weights.size() << ") that were provided in "
                      << (single_call ? paths.front() : paths[i])
                      << " file do not coincide with the number of "
                         "nodes in the block graph ("
                      << (graph.size() - 2) / devices.size() << "). I will still proceed with the import" << std::endl;
          }

        for (auto const &weight : weights)
          {
            while (extra_condition != nullptr && it != graph.cend() && !extra_condition(*it))
              {
                skip_device(it, devices.size());
              }

            if (it == graph.cend())
              break;

            for (auto const &in : graph.get_input_nodes(it->get_id()))
              graph.set_weight({in, it->get_id()}, weight);

            skip_device(it, devices.size());
          }
      }
  }
} // namespace network_butcher::io