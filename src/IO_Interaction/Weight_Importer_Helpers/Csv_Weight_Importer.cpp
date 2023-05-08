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
    if (paths.empty())
      throw std::runtime_error("No paths were provided to import the weights");

    if (paths.size() != 1 && paths.size() != relevant_entries.size())
      throw std::runtime_error(
        "The number of paths provided to import the weights is not the same as the number of the specified entries");

    if (paths.size() > devices.size())
      throw std::runtime_error(
        "The number of paths provided to import the weights is greater than the number of devices");

    if (relevant_entries.size() != devices.size())
      throw std::runtime_error(
        "The number of entries provided to import the weights is not the same as the number of devices");

    // Are we dealing with a single .csv file?
    bool const                                       single_call = paths.size() == 1;
    Weight_importer_helpers::csv_result_type<double> data;

    // Import the data (if from multiple files, append to the name of the columns a suffix)
    if (single_call)
      {
        data = Weight_importer_helpers::read_csv_numerics(paths[0], separator, relevant_entries, "", only_non_negative);
      }
    else
      {
        for (std::size_t i = 0; i < paths.size(); ++i)
          {
            auto tmp = Weight_importer_helpers::read_csv_numerics(
              paths[i], separator, {relevant_entries[i]}, "_" + std::to_string(i), only_non_negative);
            data.insert(tmp.begin(), tmp.end());
          }
      }

    // Import the weights
    for (std::size_t i = 0; i < devices.size(); ++i)
      {
        auto const &entry = single_call ? relevant_entries[i] : (relevant_entries[i] + "_" + std::to_string(i));

        auto const data_it = data.find(entry);

        if (data_it == data.cend())
          throw std::runtime_error("Missing entry for the device id " + std::to_string(i));

        auto const &values = data_it->second;
        auto        it     = ++graph.cbegin();

        for (auto const &weight : values)
          {
            while (extra_condition != nullptr && it != graph.cend() && !extra_condition(*it))
              ++it;

            if (it == graph.cend())
              break;

            for (auto const &in : graph.get_neighbors()[it->get_id()].first)
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
    if (paths.empty())
      throw std::runtime_error("No paths were provided to import the weights");

    if (paths.size() != 1 && paths.size() != relevant_entries.size())
      throw std::runtime_error(
        "The number of paths provided to import the weights is not the same as the number of the specified entries");

    if (paths.size() > devices.size())
      throw std::runtime_error(
        "The number of paths provided to import the weights is greater than the number of devices");

    if (relevant_entries.size() != devices.size())
      throw std::runtime_error(
        "The number of entries provided to import the weights is not the same as the number of devices");

    // Are we dealing with a single .csv file?
    bool const                                       single_call = paths.size() == 1;
    Weight_importer_helpers::csv_result_type<double> data;

    // Import the data (if from multiple files, append to the name of the columns a suffix)
    if (single_call)
      {
        data = Weight_importer_helpers::read_csv_numerics(paths[0], separator, relevant_entries, "", only_non_negative);
      }
    else
      {
        for (std::size_t i = 0; i < paths.size(); ++i)
          {
            auto tmp = Weight_importer_helpers::read_csv_numerics(
              paths[i], separator, {relevant_entries[i]}, "_" + std::to_string(i), only_non_negative);
            data.insert(tmp.begin(), tmp.end());
          }
      }

    // Check if there are missing entries
    for (std::size_t i = 0; i < relevant_entries.size(); ++i)
      {
        auto const &entry = relevant_entries[i];
        if (single_call ? data.find(entry) == data.cend() : data.find(entry + "_" + std::to_string(i)) == data.cend())
          {
            throw std::runtime_error("The entry " + entry + " for the device " + std::to_string(devices[i]) +
                                     " doesn't exist! We cannot import the weights");
          }
      }

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

            for (auto const &in : graph.get_neighbors()[it->get_id()].first)
              graph.set_weight({in, it->get_id()}, weight);

            skip_device(it, devices.size());
          }
      }
  }
} // namespace network_butcher::io