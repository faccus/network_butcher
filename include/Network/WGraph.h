//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_WGRAPH_H
#define NETWORK_BUTCHER_WGRAPH_H

#include "MWGraph.h"

namespace network_butcher_types
{
  template <class T>
  class WGraph : public MWGraph<T>
  {
  private:
    using Parent_type = MWGraph<T>;

  public:
    using Node_Type = typename Parent_type::Node_Type;

    WGraph()               = default;
    WGraph(WGraph const &) = default;
    WGraph &
    operator=(WGraph const &) = default;

    WGraph(WGraph &&) = default;
    WGraph &
    operator=(WGraph &&) = default;

    explicit WGraph(std::vector<Node<T>>                                                     v,
                    std::vector<std::pair<node_id_collection_type, node_id_collection_type>> dep = {})
      : Parent_type(1, v, dep)
    {}

    /// Get the weight for the given edge
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] weight_type
    get_weigth(edge_type const &edge) const
    {
      return Parent_type::get_weigth(0, edge);
    }

    /// Set the weight for the given edge
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weigth(edge_type const &edge, weight_type weight)
    {
      Parent_type::set_weigth(0, edge, weight);
    }
  };

  template <class T>
  class WGraph<Content<T>> : public MWGraph<Content<T>>
  {
  private:
    using Parent_type = MWGraph<Content<T>>;

  public:
    using Node_Type = typename Parent_type::Node_Type;

    WGraph()               = default;
    WGraph(WGraph const &) = default;
    WGraph &
    operator=(WGraph const &) = default;

    WGraph(WGraph &&) = default;
    WGraph &
    operator=(WGraph &&) = default;

    explicit WGraph(std::vector<Node<Content<T>>>                                            v,
                    std::vector<std::pair<node_id_collection_type, node_id_collection_type>> dep)
      : Parent_type(1, v, dep){};

    explicit WGraph(std::vector<Node_Type> const &v)
      : Parent_type(1, v)
    {}

    explicit WGraph(std::vector<Node_Type> &&v)
      : Parent_type(1, std::move(v))
    {}

    /// Get the weight for the given edge
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] weight_type
    get_weigth(edge_type const &edge) const
    {
      return Parent_type::get_weigth(0, edge);
    }

    /// Set the weight for the given edge
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weigth(edge_type const &edge, weight_type weight)
    {
      Parent_type::set_weigth(0, edge, weight);
    }
  };
} // namespace network_butcher_types

#endif // NETWORK_BUTCHER_WGRAPH_H
