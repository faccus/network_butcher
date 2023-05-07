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
  class Path_Converter
  {
  private:
    block_graph_type const &graph;

  public:
    explicit Path_Converter(block_graph_type const &graph)
      : graph{graph} {};

    [[nodiscard]] network_butcher::types::Weighted_Real_Paths
    convert_to_weighted_real_path(std::vector<network_butcher::kfinder::path_info> const &paths) const;

    [[nodiscard]] network_butcher::types::Weighted_Real_Path
    convert_to_weighted_real_path(network_butcher::kfinder::path_info const &paths) const;
  };
} // namespace network_butcher::Utilities

#endif // NETWORK_BUTCHER_PATH_CONVERTER_H
