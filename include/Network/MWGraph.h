//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_MWGRAPH_H
#define NETWORK_BUTCHER_MWGRAPH_H

#include "Graph.h"


namespace network_butcher_types
{
  /// A custom graph class. It contains a single graph and multiple weight maps. Technically, it can be viewed as a
  /// collection of graphs with the same structure, but different weight maps.
  /// \tparam T Type of the content of the nodes
  template <class T>
  class MWGraph : public Graph<T>
  {
  private:
    using Parent_type = Graph<T>;

  protected:
    std::vector<weights_collection_type> weigth_map;

  public:
    using Dependencies_Type    = network_butcher_types::Dependencies_Type;
    using Node_Type            = network_butcher_types::Node_Type<T>;
    using Node_Collection_Type = network_butcher_types::Node_Collection_Type<T>;
    using Node_Internal_Type   = T;

    MWGraph()                = delete;
    MWGraph(MWGraph const &) = default;
    MWGraph &
    operator=(MWGraph const &) = default;

    MWGraph(MWGraph &&) = default;
    MWGraph &
    operator=(MWGraph &&) = default;

    explicit MWGraph(std::size_t num_maps, Node_Collection_Type v, Dependencies_Type dep = {})
      : Parent_type(v, dep)
      , weigth_map{}
    {
      weigth_map.resize(num_maps);
    }


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] weight_type
    get_weight(std::size_t device, edge_type const &edge) const
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
    set_weight(std::size_t device, edge_type const &edge, weight_type weight)
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
  private:
    using Parent_type = Graph<Content<T>>;

  protected:
    std::vector<weights_collection_type> weigth_map;

  public:
    using Dependencies_Type    = network_butcher_types::Dependencies_Type;
    using Node_Type            = network_butcher_types::Node_Type<Content<T>>;
    using Node_Collection_Type = network_butcher_types::Node_Collection_Type<Content<T>>;
    using Node_Internal_Type   = T;
    using Node_Content_Type    = Content<T>;

    MWGraph()                = delete;
    MWGraph(MWGraph const &) = default;
    MWGraph &
    operator=(MWGraph const &) = default;

    MWGraph(MWGraph &&) = default;
    MWGraph &
    operator=(MWGraph &&) = default;

    explicit MWGraph(std::size_t num_maps, Node_Collection_Type v, Dependencies_Type dep)
      : Parent_type(v, dep)
      , weigth_map{}
    {
      weigth_map.resize(num_maps);
    }

    explicit MWGraph(std::size_t num_maps, Node_Collection_Type const &v)
      : Parent_type(v)
      , weigth_map{}
    {
      weigth_map.resize(num_maps);
    }

    explicit MWGraph(std::size_t num_maps, Node_Collection_Type &&v)
      : Parent_type(std::move(v))
      , weigth_map{}
    {
      weigth_map.resize(num_maps);
    }


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] weight_type
    get_weight(std::size_t device, edge_type const &edge) const
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
    set_weight(std::size_t device, edge_type const &edge, weight_type weight)
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
