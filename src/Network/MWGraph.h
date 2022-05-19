//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_MWGRAPH_H
#define NETWORK_BUTCHER_MWGRAPH_H

#include "Graph.h"

template <class T>
class MWGraph : public Graph<T>
{
protected:
  std::vector<weights_collection_type> weigth_map;

public:
  MWGraph()                = default;
  MWGraph(MWGraph const &) = default;
  MWGraph &
  operator=(MWGraph const &) = default;

  MWGraph(MWGraph &&) = default;
  MWGraph &
  operator=(MWGraph &&) = default;

  explicit MWGraph(
    std::size_t          num_maps,
    std::vector<Node<T>> v,
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      dep = {})
    : Graph<T>(v, dep)
    , weigth_map{}
  {
    weigth_map.resize(num_maps);
  }


  [[nodiscard]] weight_type const &
  get_weigth(std::size_t index, edge_type const &edge) const
  {
    auto const &map = weigth_map[index];
    auto const  p   = map.find(edge);
    if (p == map.cend())
      return -1.;
    else
      return p->second;
  }

  void
  set_weigth(std::size_t index, edge_type const &edge, weight_type weight)
  {
    weigth_map[index][edge] = weight;
  }
};

template <class T>
class MWGraph<Content<T>> : public Graph<Content<T>>
{
protected:
  std::vector<weights_collection_type> weigth_map;
  using Node_Type = typename Graph<Content<T>>::Node_Type;

public:
  MWGraph()                = default;
  MWGraph(MWGraph const &) = default;
  MWGraph &
  operator=(MWGraph const &) = default;

  MWGraph(MWGraph &&) = default;
  MWGraph &
  operator=(MWGraph &&) = default;

  explicit MWGraph(
    std::size_t                   num_maps,
    std::vector<Node<Content<T>>> v,
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      dep)
    : Graph<Content<T>>(v, dep)
    , weigth_map{}
  {
    weigth_map.resize(num_maps);
  }

  explicit MWGraph(std::size_t num_maps, std::vector<Node_Type> const &v)
    : Graph<Content<T>>(v)
    , weigth_map{}
  {
    weigth_map.resize(num_maps);
  }

  explicit MWGraph(std::size_t num_maps, std::vector<Node_Type> &&v)
    : Graph<Content<T>>(std::move(v))
    , weigth_map{}
  {
    weigth_map.resize(num_maps);
  }


  [[nodiscard]] weight_type const &
  get_weigth(std::size_t index, edge_type const &edge) const
  {
    auto const &map = weigth_map[index];
    auto const  p   = map.find(edge);
    if (p == map.cend())
      return -1.;
    else
      return p->second;
  }

  void
  set_weigth(std::size_t index, edge_type const &edge, weight_type weight)
  {
    weigth_map[index][edge] = weight;
  }
};

#endif // NETWORK_BUTCHER_MWGRAPH_H
