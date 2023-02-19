//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_WGRAPH_H
#define NETWORK_BUTCHER_WGRAPH_H

#include "MWGraph.h"
#include "Weighted_Graph.h"

namespace network_butcher_types
{
  /// Just another weighted graph class...
  /// \tparam T Type of the content of the nodes
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
    get_weight(edge_type const &edge) const
    {
      return Parent_type::get_weight(0, edge);
    }

    /// Set the weight for the given edge
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(edge_type const &edge, weight_type weight)
    {
      Parent_type::set_weight(0, edge, weight);
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
    get_weight(edge_type const &edge) const
    {
      return Parent_type::get_weight(0, edge);
    }

    /// Set the weight for the given edge
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(edge_type const &edge, weight_type weight)
    {
      Parent_type::set_weight(0, edge, weight);
    }
  };

} // namespace network_butcher_types

namespace network_butcher_kfinder {

  template <class T>
  class Weighted_Graph<network_butcher_types::WGraph<T>>  {

  public:
    using Node_Id_Type = std::size_t;
    using Edge_Type = std::pair<Node_Id_Type, Node_Id_Type>;
    using Graph_Type = network_butcher_types::WGraph<T>;
    using Node_Type = Graph_Type::Node_Type;


    weight_type
    get_weight(Edge_Type const &edge) const {
      return graph.get_weight(edge);
    }

    std::size_t
    size() const {
      return graph.size();
    };

    bool
    empty() const {
      return graph.empty();
    };



    std::set<Node_Id_Type>
    get_input_nodes(Node_Id_Type const &id) const {
      return graph.get_neighbors()[id].first;
    };

    std::set<Node_Id_Type>
    get_output_nodes(Node_Id_Type const &id) const{
      return graph.get_neighbors()[id].second;
    };

    std::vector<std::pair<std::set<Node_Id_Type>, std::set<Node_Id_Type>>> const &
    get_neighbors() const {
      return graph.get_neighbors();
    };



    std::vector<Node_Type> const &
      get_nodes() const {
      return graph.get_nodes();
    }

    Node_Type const &
    operator[](Node_Id_Type const &id) const{
      return graph[id];
    };




    Weighted_Graph(Graph_Type &g) : graph(g) {}

  private:
    Graph_Type const &graph;
  };
}


#endif // NETWORK_BUTCHER_WGRAPH_H
