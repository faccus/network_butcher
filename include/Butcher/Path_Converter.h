//
// Created by faccus on 24/04/23.
//

#ifndef NETWORK_BUTCHER_PATH_CONVERTER_H
#define NETWORK_BUTCHER_PATH_CONVERTER_H

#include "Graph_traits.h"
#include "Path_info.h"
#include "Paths.h"

namespace network_butcher::Utilities
{
  /// Simple class used to convert the output of the K-shortest path algorithms to a Weighted_Real_Paths
  template <typename Weight_Type = weight_type>
  class Path_Converter
  {
  private:
    block_graph_type const &graph;

  public:
    explicit Path_Converter(block_graph_type const &graph)
      : graph{graph} {};

    [[nodiscard]] network_butcher::types::Weighted_Real_Paths
    convert_to_weighted_real_path(std::vector<network_butcher::kfinder::t_path_info<Weight_Type>> const &paths) const;

    [[nodiscard]] network_butcher::types::Weighted_Real_Path
    convert_to_weighted_real_path(network_butcher::kfinder::t_path_info<Weight_Type> const &path) const;
  };

  template <typename Weight_Type>
  network_butcher::types::Weighted_Real_Path
  Path_Converter<Weight_Type>::convert_to_weighted_real_path(const kfinder::t_path_info<Weight_Type> &path) const
  {
    return convert_to_weighted_real_path({path});
  }

  template <typename Weight_Type>
  network_butcher::types::Weighted_Real_Paths
  Path_Converter<Weight_Type>::convert_to_weighted_real_path(
    const std::vector<network_butcher::kfinder::t_path_info<Weight_Type>> &paths) const
  {
    network_butcher::types::Weighted_Real_Paths final_res(paths.size());

    std::transform(PAR_UNSEQ, paths.cbegin(), paths.cend(), final_res.begin(), [&graph = graph](auto const &path) {
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
    });

    return final_res;
  }
} // namespace network_butcher::Utilities

#endif // NETWORK_BUTCHER_PATH_CONVERTER_H
