//
// Created by faccus on 24/04/23.
//
#include "Path_Converter.h"

namespace network_butcher::Utilities
{
  network_butcher::types::Weighted_Real_Path
  Path_Converter::convert_to_weighted_real_path(network_butcher::kfinder::path_info const &path) const
  {
    network_butcher::types::Real_Path res;
    std::size_t                       current_model_device = 0;

    res.emplace_back(current_model_device, std::set<node_id_type>());

    auto const &path_nodes = path.path;

    for (auto const &node_id_new_graph : path_nodes)
      {
        auto const &node = graph[node_id_new_graph];

        if (node.content.first != current_model_device)
          {
            current_model_device = node.content.first;
            res.emplace_back(current_model_device, std::set<node_id_type>());
          }

        res.back().second.insert(node.content.second->begin(), node.content.second->end());
      }

    return network_butcher::types::Weighted_Real_Path{path.length, res};
  }

  network_butcher::types::Weighted_Real_Paths
  Path_Converter::convert_to_weighted_real_path(std::vector<network_butcher::kfinder::path_info> const &paths) const
  {
    network_butcher::types::Weighted_Real_Paths final_res(paths.size());

    std::transform(paths.cbegin(), paths.cend(), final_res.begin(), [this](auto const &path) {
      return convert_to_weighted_real_path(path);
    });

    return final_res;
  }
} // namespace network_butcher::Utilities