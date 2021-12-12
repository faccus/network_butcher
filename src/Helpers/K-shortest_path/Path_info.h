//
// Created by faccus on 12/12/21.
//

#ifndef NETWORK_BUTCHER_PATH_INFO_H
#define NETWORK_BUTCHER_PATH_INFO_H


#include "../Traits/Graph_traits.h"

struct path_info
{
  type_weight               length;
  std::vector<node_id_type> path;

  constexpr bool
  operator<(const path_info &rhs) const
  {
    return length < rhs.length || (length == rhs.length && path < rhs.path);
  }
};

struct implicit_path_info
{
  std::vector<edge_type> sidetracks;
  type_weight            length;

  constexpr bool
  operator<(const implicit_path_info &rhs) const
  {
    return length < rhs.length ||
           (length == rhs.length && sidetracks < rhs.sidetracks);
  }
};

/// A helper struct for the dijkstra algo
struct dijkstra_helper_struct
{
  type_weight  weight;
  node_id_type id;

  constexpr bool
  operator<(const dijkstra_helper_struct &rhs) const
  {
    return weight < rhs.weight || (weight == rhs.weight && id < rhs.id);
  }
};

using dijkstra_result_type =
  std::pair<std::vector<node_id_type>, std::vector<type_weight>>;

#endif // NETWORK_BUTCHER_PATH_INFO_H
