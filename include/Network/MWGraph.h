//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_MWGRAPH_H
#define NETWORK_BUTCHER_MWGRAPH_H

#include "Graph.h"


namespace network_butcher_types
{
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


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] weight_type
    get_weigth(std::size_t device, edge_type const &edge) const
    {
      auto const &map = weigth_map[device];
      auto const  p   = map.find(edge);
      if (p == map.cend())
        return 0.;
      else
        return p->second;
    }

    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weigth(std::size_t device, edge_type const &edge, weight_type weight)
    {
      weigth_map[device][edge] = weight;
    }

    /// Gets the number of devices
    /// \return Number of devices
    [[nodiscard]] std::size_t
    get_num_devices() const
    {
      return weigth_map.size();
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


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] weight_type
    get_weigth(std::size_t device, edge_type const &edge) const
    {
      auto const &map = weigth_map[device];
      auto const  p   = map.find(edge);
      if (p == map.cend())
        return 0.;
      else
        return p->second;
    }

    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weigth(std::size_t device, edge_type const &edge, weight_type weight)
    {
      weigth_map[device][edge] = weight;
    }

    /// Gets the number of devices
    /// \return Number of devices
    [[nodiscard]] std::size_t
    get_num_devices() const
    {
      return weigth_map.size();
    }

    /// Gets the weight maps
    /// \return The weight maps
    [[nodiscard]] std::vector<weights_collection_type> const &
    get_weight_map() const
    {
      return weigth_map;
    }
  };
} // namespace network_butcher_types

#endif // NETWORK_BUTCHER_MWGRAPH_H
