//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_WGRAPH_H
#define NETWORK_BUTCHER_WGRAPH_H

#include "Graph.h"

template <class T>
class WGraph : public Graph<T>
{
protected:
  weights_collection_type weigth_map;

public:

  WGraph() = default;
  WGraph(WGraph const &) = default;
  WGraph & operator = (WGraph const &) = default;

  WGraph(WGraph &&) = default;
  WGraph & operator = (WGraph &&) = default;

  explicit WGraph(
    std::vector<Node<T>> v,
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      dep = {}) : Graph<T>(v, dep) {}


  [[nodiscard]] weight_type
  get_weigth(edge_type const &edge) const
  {
    auto const p = weigth_map.find(edge);
    if (p == weigth_map.cend())
      return -1.;
    else
      return p->second;
  }

  void
  set_weigth(edge_type const &edge, weight_type weight)
  {
    weigth_map[edge] = weight;
  }
};

template <class T>
class WGraph<Content<T>> : public Graph<Content<T>>
{
protected:
  weights_collection_type weigth_map;
  using Node_Type = typename Graph<Content<T>>::Node_Type;

public:

  WGraph() = default;
  WGraph(WGraph const &) = default;
  WGraph & operator = (WGraph const &) = default;

  WGraph(WGraph &&) = default;
  WGraph & operator = (WGraph &&) = default;

  explicit WGraph(
    std::vector<Node<Content<T>>> v,
    std::vector<std::pair<node_id_collection_type, node_id_collection_type>>
      dep) : Graph<Content<T>>(v, dep) {};

  explicit WGraph(std::vector<Node_Type> const &v)
    : Graph<Content<T>>(v) {}

  explicit WGraph(std::vector<Node_Type> &&v)
    : Graph<Content<T>>(std::move(v)) {}


  [[nodiscard]] weight_type
  get_weigth(edge_type const &edge) const
  {
    auto const p = weigth_map.find(edge);
    if (p == weigth_map.cend())
      return -1.;
    else
      return p->second;
  }

  void
  set_weigth(edge_type const &edge, weight_type weight)
  {
    weigth_map[edge] = weight;
  }
};

#endif // NETWORK_BUTCHER_WGRAPH_H
