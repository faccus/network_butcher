//
// Created by faccus on 13/04/23.
//
#include "Csv_Weight_Importer.h"

namespace network_butcher::io
{
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

  void
  Csv_Weight_Importer<graph_type>::import_weights()
  {
    import_weights(nullptr);
  }
} // namespace network_butcher::io