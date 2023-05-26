//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_MWGRAPH_H
#define NETWORK_BUTCHER_MWGRAPH_H

#include "Graph.h"


namespace network_butcher::types
{
  /// A custom graph class. It contains a single graph and multiple weight maps. Technically, it can be viewed
  /// as a collection of graphs with the same structure, but different weight maps.
  /// \tparam T Type of the content of the nodes
  template <bool Parallel_Edges, typename t_Node_Type = Node, typename t_weight_type = weight_type>
  class MWGraph : public Graph<t_Node_Type>
  {
  private:
    using Parent_type = Graph<t_Node_Type>;

  public:
    using Dependencies_Type    = Parent_type::Neighbours_Type;
    using Node_Type            = Parent_type::Node_Type;
    using Node_Collection_Type = Parent_type::Node_Collection_Type;

    using Weight_Type = t_weight_type;

    using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<Weight_Type>, Weight_Type>;
    using Weight_Collection_Type =
      std::conditional_t<Parallel_Edges, std::multimap<edge_type, Weight_Type>, std::map<edge_type, Weight_Type>>;

  protected:
    std::vector<Weight_Collection_Type> weigth_map;

  public:
    template <typename A, typename B>
    explicit MWGraph(std::size_t num_maps, A &&v, B &&dep)
      : Parent_type(std::forward<A>(v), std::forward<B>(dep))
      , weigth_map{}
    {
      weigth_map.resize(num_maps);
    }


    /// Checks if the given edge has a weight on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] bool
    check_weight(std::size_t device, edge_type const &edge) const
    {
      return weigth_map[device].contains(edge);
    }


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] auto
    get_weight(std::size_t device, edge_type const &edge) const
    {
      auto const &map   = weigth_map[device];
      auto [begin, end] = map.equal_range(edge);
      Edge_Weight_Type res;

      if constexpr (Parallel_Edges)
        {
          for (; begin != end; ++begin)
            res.insert(begin->second);
        }
      else
        {
          if (begin != end)
            res = begin->second;
        }

      return res;
    }


    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weights The weight
    void
    set_weight(std::size_t device, edge_type const &edge, Edge_Weight_Type weights)
      requires Parallel_Edges
    {
      for (auto const &weight : weights)
        weigth_map[device].emplace(edge, weight);
    }

    void
    delete_weight(std::size_t device, edge_type const &edge)
    {
      auto [begin, end] = weigth_map[device].equal_range(edge);
      weigth_map[device].erase(begin, end);
    }

    void
    delete_weight(edge_type const &edge)
    {
      for (std::size_t i = 0; i < weigth_map.size(); ++i)
        delete_weight(i, edge);
    }


    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(std::size_t device, edge_type const &edge, t_weight_type weight)
    {
      if constexpr (Parallel_Edges)
        {
          weigth_map[device].emplace(edge, weight);
        }
      else
        {
          weigth_map[device][edge] = weight;
        }
    }

    /// Gets the number of devices
    /// \return Number of devices
    [[nodiscard]] std::size_t
    get_num_devices() const
    {
      return weigth_map.size();
    }

    ~MWGraph() override = default;
  };

  /// A custom graph class. It contains a single graph and multiple weight maps. Technically, it can be viewed
  /// as a collection of graphs with the same structure, but different weight maps.
  /// \tparam T Type of the content of the nodes
  template <bool Parallel_Edges, typename T>
  class MWGraph<Parallel_Edges, CNode<Content<T>>, weight_type> : public Graph<CNode<Content<T>>>
  {
  private:
    using Parent_type = Graph<CNode<Content<T>>>;

  public:
    using Dependencies_Type    = Parent_type::Neighbours_Type;
    using Node_Type            = Parent_type::Node_Type;
    using Node_Collection_Type = Parent_type::Node_Collection_Type;

    using Weight_Type = weight_type;

    using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<weight_type>, weight_type>;
    using Weight_Collection_Type =
      std::conditional_t<Parallel_Edges, std::multimap<edge_type, weight_type>, std::map<edge_type, weight_type>>;

  protected:
    std::vector<Weight_Collection_Type> weigth_map;

  public:
    template <typename A, typename B>
    explicit MWGraph(std::size_t num_maps, A &&v, B &&dep)
      : Parent_type(std::forward<A>(v), std::forward<B>(dep))
      , weigth_map{}
    {
      weigth_map.resize(num_maps);
    }

    template <typename A>
    explicit MWGraph(std::size_t num_maps, A &&v)
      : Parent_type(std::forward<A>(v))
      , weigth_map{}
    {
      weigth_map.resize(num_maps);
    }


    /// Checks if the given edge has a weight on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] bool
    check_weight(std::size_t device, edge_type const &edge) const
    {
      return weigth_map[device].contains(edge);
    }


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] Edge_Weight_Type
    get_weight(std::size_t device, edge_type const &edge) const
    {
      auto const &map   = weigth_map[device];
      auto [begin, end] = map.equal_range(edge);
      Edge_Weight_Type res;

      if constexpr (Parallel_Edges)
        {
          for (; begin != end; ++begin)
            res.insert(begin->second);
        }
      else
        {
          if (begin != end)
            res = begin->second;
        }

      return res;
    }


    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weights The weights
    void
    set_weight(std::size_t device, edge_type const &edge, Edge_Weight_Type weights)
      requires Parallel_Edges
    {
      for (auto const &weight : weights)
        weigth_map[device].emplace(edge, weight);
    }


    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(std::size_t device, edge_type const &edge, weight_type weight)
    {
      if constexpr (Parallel_Edges)
        {
          weigth_map[device].emplace(edge, weight);
        }
      else
        {
          weigth_map[device][edge] = weight;
        }
    }

    void
    delete_weight(std::size_t device, edge_type const &edge)
    {
      auto [begin, end] = weigth_map[device].equal_range(edge);
      weigth_map[device].erase(begin, end);
    }

    void
    delete_weight(edge_type const &edge)
    {
      for (std::size_t i = 0; i < weigth_map.size(); ++i)
        delete_weight(i, edge);
    }

    /// Gets the number of devices
    /// \return Number of devices
    [[nodiscard]] std::size_t
    get_num_devices() const
    {
      return weigth_map.size();
    }

    ~MWGraph() override = default;
  };
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_MWGRAPH_H
