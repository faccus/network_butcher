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
    if (paths.empty() || (paths.size() != 1 && paths.size() != relevant_entries.size()) ||
        paths.size() > devices.size() || relevant_entries.size() != devices.size())
      throw;

    bool const single_call = paths.size() == 1;

    Weight_importer_helpers::csv_result_type<double> data;

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

    for (std::size_t i = 0; i < devices.size(); ++i)
      {
        auto const &device = devices[i];
        auto const &entry  = single_call ? relevant_entries[i] : (relevant_entries[i] + "_" + std::to_string(i));

        auto const data_it = data.find(entry);

        if (data_it == data.cend())
          throw; // ("Missing entry for device Id " + std::to_string(i));

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
    if (paths.empty() || (paths.size() != 1 && paths.size() != relevant_entries.size()) ||
        paths.size() > devices.size() || relevant_entries.size() != devices.size())
      throw;

    bool const single_call = paths.size() == 1;

    Weight_importer_helpers::csv_result_type<double> data;

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

    for (std::size_t i = 0; i < devices.size(); ++i)
      {
        auto const &device = devices[i];

        auto        it = graph.cbegin();
        auto const &weights_it =
          single_call ? data.find(relevant_entries[i]) : data.find(relevant_entries[i] + "_" + std::to_string(i));

        if (weights_it == data.cend())
          throw std::runtime_error("The entry " + relevant_entries[i] + " doesn't exist! We cannot import the weights");

        for (std::size_t j = 0; it != graph.cend() && j < (device + 1); ++j, ++it)
          ;

        for (auto const &weight : weights_it->second)
          {
            while (extra_condition != nullptr && it != graph.cend() && !extra_condition(*it))
              {
                for (std::size_t j = 0; it != graph.cend() && j < devices.size(); ++j, ++it)
                  ;
              }

            if (it == graph.cend())
              break;

            for (auto const &in : graph.get_neighbors()[it->get_id()].first)
              graph.set_weight({in, it->get_id()}, weight);

            for (std::size_t j = 0; it != graph.cend() && j < devices.size(); ++j, ++it)
              ;
          }
      }
  }
} // namespace network_butcher::io