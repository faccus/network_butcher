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
    /// It will prepare a Path_Converter
    /// \param graph A const reference to a block graph
    explicit Path_Converter(block_graph_type const &graph)
      : graph{graph} {};

    /// It will convert a collection of paths of the block graph to a partitioning
    /// \param paths The collection of paths
    /// \return The different partitioning
    [[nodiscard]] std::vector<network_butcher::types::Weighted_Real_Path>
    convert_to_weighted_real_path(std::vector<network_butcher::kfinder::t_path_info<Weight_Type>> const &paths) const;

    /// It will convert a path of the block graph to a partitioning
    /// \param path The path
    /// \return The related partitioning
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
  std::vector<network_butcher::types::Weighted_Real_Path>
  Path_Converter<Weight_Type>::convert_to_weighted_real_path(
    const std::vector<network_butcher::kfinder::t_path_info<Weight_Type>> &paths) const
  {
    std::vector<network_butcher::types::Weighted_Real_Path> final_res(paths.size());

    auto func = [&graph = graph](auto const &path) {
      network_butcher::types::Real_Path res;
      std::size_t                       current_model_device = 0;

      res.emplace_back(current_model_device, std::set<node_id_type>());

      auto const &path_nodes = path.path;

      // Loop through the nodes of the path
      for (auto const &node_id_new_graph : path_nodes)
        {
          auto const &node = graph[node_id_new_graph];

          // Check if a new device is requested
          if (node.content.first != current_model_device)
            {
              // Add a new partition
              current_model_device = node.content.first;
              res.emplace_back(current_model_device, std::set<node_id_type>());
            }

          // Add the current node to the last partition
          res.back().second.insert(node.content.second->begin(), node.content.second->end());
        }

      return network_butcher::types::Weighted_Real_Path{path.length, res};
    };

    // Process the different paths into partitioning
    Utilities::potentially_par_transform(paths.cbegin(), paths.cend(), final_res.begin(), func);

    return final_res;
  }
} // namespace network_butcher::Utilities

#endif // NETWORK_BUTCHER_PATH_CONVERTER_H
