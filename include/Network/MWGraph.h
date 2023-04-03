//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_MWGRAPH_H
#define NETWORK_BUTCHER_MWGRAPH_H

#include "Graph.h"


namespace network_butcher
{
  namespace types
  {
    /// A custom graph class. It contains a single graph and multiple weight maps. Technically, it can be viewed
    /// as a collection of graphs with the same structure, but different weight maps.
    /// \tparam T Type of the content of the nodes
    template <typename T>
    class MWGraph : public Graph<T>
    {
    private:
      using Parent_type = Graph<T>;

    protected:
      std::vector<weights_collection_type> weigth_map;

    public:
      using Dependencies_Type    = network_butcher::types::Dependencies_Type;
      using Node_Type            = network_butcher::types::Node_Type<T>;
      using Node_Collection_Type = network_butcher::types::Node_Collection_Type<T>;
      using Node_Internal_Type   = T;

      friend std::ostream &
      operator<<(std::ostream &os, MWGraph<T> const &g)
      {
        os << "Number of nodes: " << g.size() << std::endl;
        os << "Weights: " << std::endl;

        std::map<edge_type, std::vector<std::optional<weight_type>>> edge_map;
        for (std::size_t i = 0; i < g.weigth_map.size(); ++i)
          {
            auto const &weight_map = g.weigth_map[i];

            for (auto const &[edge, weight] : weight_map)
              {
                edge_map[edge].resize(g.weigth_map.size());
                edge_map[edge][i] = weight;
              }
          }

        for (auto const &[edge, weights] : edge_map)
          {
            os << "(" << edge.first << ", " << edge.second << ") : ";
            for (auto const &weight : weights)
              {
                if (weight.has_value())
                  os << weight.value() << " ";
                else
                  os << "N/A ";
              }
            os << std::endl;
          }

        return os;
      }

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

      [[nodiscard]] bool
      check_weight(std::size_t device, edge_type const &edge) const
      {
        auto const &map = weigth_map[device];
        return map.find(edge) != map.cend();
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

    /// A custom graph class. It contains a single graph and multiple weight maps. Technically, it can be viewed
    /// as a collection of graphs with the same structure, but different weight maps.
    /// \tparam T Type of the content of the nodes
    template <typename T>
    class MWGraph<Content<T>> : public Graph<Content<T>>
    {
    private:
      using Parent_type = Graph<Content<T>>;

    protected:
      std::vector<weights_collection_type> weigth_map;

    public:
      using Dependencies_Type    = network_butcher::types::Dependencies_Type;
      using Node_Type            = network_butcher::types::Node_Type<Content<T>>;
      using Node_Collection_Type = network_butcher::types::Node_Collection_Type<Content<T>>;
      using Node_Internal_Type   = T;
      using Node_Content_Type    = Content<T>;

      friend std::ostream &
      operator<<(std::ostream &os, MWGraph<Content<T>> const &g)
      {
        os << "Number of nodes: " << g.size() << std::endl;
        os << "Weights: " << std::endl;

        std::map<edge_type, std::vector<std::optional<weight_type>>> edge_map;
        for (std::size_t i = 0; i < g.weigth_map.size(); ++i)
          {
            auto const &weight_map = g.weigth_map[i];

            for (auto const &[edge, weight] : weight_map)
              {
                edge_map[edge].resize(g.weigth_map.size());
                edge_map[edge][i] = weight;
              }
          }

        for (auto const &[edge, weights] : edge_map)
          {
            os << "(" << edge.first << ", " << edge.second << ") : ";
            for (auto const &weight : weights)
              {
                if (weight.has_value())
                  os << weight.value() << " ";
                else
                  os << "N/A ";
              }
            os << std::endl;
          }

        return os;
      }

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

      [[nodiscard]] bool
      check_weight(std::size_t device, edge_type const &edge) const
      {
        auto const &map = weigth_map[device];
        return map.find(edge) != map.cend();
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

  } // namespace types

} // namespace network_butcher

#endif // NETWORK_BUTCHER_MWGRAPH_H
