//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_WGRAPH_H
#define NETWORK_BUTCHER_WGRAPH_H

#include "MWGraph.h"
#include "Weighted_Graph.h"


namespace network_butcher::types
{
  /// Just another weighted graph class...
  /// \tparam Parallel_Edges If true, the graph will support parallel edges
  /// \tparam t_Node_Type Type of nodes
  /// \tparam t_weight_type Type of the weight
  template <bool Parallel_Edges, typename t_Node_Type = Node, typename t_weight_type = weight_type>
  class WGraph : public Graph<t_Node_Type>
  {
  private:
    using Parent_Type  = Graph<t_Node_Type>;
    using MWGraph_Type = MWGraph<Parallel_Edges, t_Node_Type, t_weight_type>;

    MWGraph_Type::Weight_Collection_Type weigth_map;

  public:
    using Dependencies_Type    = MWGraph_Type::Dependencies_Type;
    using Node_Type            = MWGraph_Type::Node_Type;
    using Node_Collection_Type = MWGraph_Type::Node_Collection_Type;

    using Weight_Type            = MWGraph_Type::Weight_Type;
    using Edge_Weight_Type       = MWGraph_Type::Edge_Weight_Type;
    using Weight_Collection_Type = MWGraph_Type::Weight_Collection_Type;

    template <typename A, typename B>
    explicit WGraph(A &&v, B &&dep)
      : Parent_Type(std::forward<A>(v), std::forward<B>(dep))
      , weigth_map{}
    {}


    /// Checks if the given edge has a weight
    /// \param device The device id
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] bool
    check_weight(edge_type const &edge) const
    {
      return weigth_map.contains(edge);
    }


    /// Get the weight for the given edge
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] auto
    get_weight(edge_type const &edge) const
    {
      auto const      &map = weigth_map;
      auto             it  = map.find(edge);
      Edge_Weight_Type res;

      if (it == map.cend())
        {
          throw std::runtime_error("WGraph: the edge " + Utilities::custom_to_string(edge) +
                                   " was not associated with any weight");
        }

      if constexpr (Parallel_Edges)
        {
          for (; it != map.cend() && it->first == edge; ++it)
            res.insert(it->second);
        }
      else
        {
          res = it->second;
        }

      return res;
    }


    /// Sets the weight for the given edge
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(edge_type const &edge, t_weight_type const &weight)
    {
      if (!Parent_Type::check_edge(edge))
        {
          std::cout << "WGraph: the edge " << Utilities::custom_to_string(edge) << " is not in the graph" << std::endl;
          return;
        }

      if constexpr (Parallel_Edges)
        {
          weigth_map.emplace(edge, weight);
        }
      else
        {
          weigth_map[edge] = weight;
        }
    }


    /// Sets the weight for the given edge
    /// \param edge The edge
    /// \param weights The weight
    void
    set_weight(edge_type const &edge, Edge_Weight_Type weights)
      requires Parallel_Edges
    {
      if (Parent_Type::check_edge(edge))
        {
          for (auto const &weight : weights)
            weigth_map.emplace(edge, weight);
        }
      else
        {
          std::cout << "WGraph: the edge " << Utilities::custom_to_string(edge) << " is not in the graph" << std::endl;
        }
    }


    /// Simple helper function that will print the graph
    /// \return The graph description
    [[nodiscard]] decltype(auto)
    print_graph() const
    {
      std::stringstream builder;
      builder << "In Out Weight" << std::endl;

      for (auto const &node : Parent_Type::nodes)
        {
          for (auto const &out : Parent_Type::get_output_nodes(node.get_id()))
            {
              auto const  edge     = std::make_pair(node.get_id(), out);
              std::string base_tmp = Utilities::custom_to_string(edge) + " ";

              if (check_weight(edge))
                {
                  if constexpr (Parallel_Edges)
                    {
                      for (auto const &w : get_weight(edge))
                        {
                          builder << base_tmp << Utilities::custom_to_string(w) << std::endl;
                        }
                    }
                  else
                    {
                      builder << base_tmp << Utilities::custom_to_string(get_weight(edge)) << std::endl;
                    }
                }
              else
                {
                  builder << base_tmp << "$" << std::endl;
                }
            }
        }
      return builder.str();
    }


    ~WGraph() override = default;
  };


  /// A graph with weights
  /// \tparam Parallel_Edges If true, the graph will support parallel edges
  /// \tparam T The type of the content of the nodes
  template <bool Parallel_Edges, typename T>
  class WGraph<Parallel_Edges, CNode<Content<T>>> : public Graph<CNode<Content<T>>>
  {
  private:
    using t_Node_Type   = CNode<Content<T>>;
    using t_weight_type = weight_type;

    using Parent_Type  = Graph<t_Node_Type>;
    using MWGraph_Type = MWGraph<Parallel_Edges, t_Node_Type, t_weight_type>;

    MWGraph_Type::Weight_Collection_Type weigth_map;

  public:
    using Dependencies_Type    = MWGraph_Type::Dependencies_Type;
    using Node_Type            = MWGraph_Type::Node_Type;
    using Node_Collection_Type = MWGraph_Type::Node_Collection_Type;

    using Weight_Type            = MWGraph_Type::Weight_Type;
    using Edge_Weight_Type       = MWGraph_Type::Edge_Weight_Type;
    using Weight_Collection_Type = MWGraph_Type::Weight_Collection_Type;

    template <typename A, typename B>
    explicit WGraph(A &&v, B &&dep)
      : Parent_Type(std::forward<A>(v), std::forward<B>(dep))
      , weigth_map{}
    {}

    explicit WGraph(Node_Collection_Type const &v)
      : Parent_Type(v)
    {}
    explicit WGraph(Node_Collection_Type &&v)
      : Parent_Type(std::move(v))
    {}


    /// Checks if the given edge has a weight
    /// \param device The device id
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] bool
    check_weight(edge_type const &edge) const
    {
      return weigth_map.contains(edge);
    }


    /// Get the weight for the given edge
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] auto
    get_weight(edge_type const &edge) const
    {
      auto const      &map = weigth_map;
      auto             it  = map.find(edge);
      Edge_Weight_Type res;

      if (it == map.cend())
        {
          throw std::runtime_error("WGraph: the edge " + Utilities::custom_to_string(edge) +
                                   " was not associated with any weight");
        }

      if constexpr (Parallel_Edges)
        {
          for (; it != map.cend() && it->first == edge; ++it)
            res.insert(it->second);
        }
      else
        {
          res = it->second;
        }

      return res;
    }


    /// Sets the weight for the given edge
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(edge_type const &edge, t_weight_type const &weight)
    {
      if (!Parent_Type::check_edge(edge))
        {
          std::cout << "WGraph: the edge " << Utilities::custom_to_string(edge) << " is not in the graph" << std::endl;
          return;
        }

      if constexpr (Parallel_Edges)
        {
          weigth_map.emplace(edge, weight);
        }
      else
        {
          weigth_map[edge] = weight;
        }
    }


    /// Sets the weight for the given edge
    /// \param edge The edge
    /// \param weights The weight
    void
    set_weight(edge_type const &edge, Edge_Weight_Type weights)
      requires Parallel_Edges
    {
      if (Parent_Type::check_edge(edge))
        {
          for (auto const &weight : weights)
            weigth_map.emplace(edge, weight);
        }
      else
        {
          std::cout << "WGraph: the edge " << Utilities::custom_to_string(edge) << " is not in the graph" << std::endl;
        }
    }


    /// Simple helper function that will print the graph
    /// \return The graph description
    [[nodiscard]] decltype(auto)
    print_graph() const
    {
      std::stringstream builder;
      builder << "In Out Weight" << std::endl;

      for (auto const &node : Parent_Type::nodes)
        {
          for (auto const &out : Parent_Type::get_output_nodes(node.get_id()))
            {
              auto const  edge     = std::make_pair(node.get_id(), out);
              std::string base_tmp = Utilities::custom_to_string(edge) + " ";

              if (check_weight(edge))
                {
                  if constexpr (Parallel_Edges)
                    {
                      for (auto const &w : get_weight(edge))
                        {
                          builder << base_tmp << Utilities::custom_to_string(w) << std::endl;
                        }
                    }
                  else
                    {
                      builder << base_tmp << Utilities::custom_to_string(get_weight(edge)) << std::endl;
                    }
                }
              else
                {
                  builder << base_tmp << "$" << std::endl;
                }
            }
        }
      return builder.str();
    }


    ~WGraph() override = default;
  };
} // namespace network_butcher::types

namespace network_butcher::kfinder
{
  /// (Partial) Specialization of Weighted_Graph
  /// \tparam T The internal type of WGraph
  template <bool Parallel_Edges, typename t_Node_Type, bool t_Reversed>
  class Weighted_Graph<network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>,
                       t_Reversed,
                       typename network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>::Node_Type,
                       typename network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>::Node_Collection_Type,
                       weight_type> : base_Weighted_Graph
  {
  public:
    using Weight_Type = weight_type;

    using Edge_Type = std::pair<node_id_type, node_id_type>;

    using Graph_Type       = network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>;
    using Weight_Edge_Type = std::multiset<Weight_Type>;

    using Node_Type            = t_Node_Type;
    using Node_Collection_Type = std::vector<Node_Type>;


    [[nodiscard]] Weight_Edge_Type
    get_weight(Edge_Type const &edge) const
    {
      if constexpr (t_Reversed && Parallel_Edges)
        {
          return graph.get_weight(std::make_pair(edge.second, edge.first));
        }
      else if constexpr (t_Reversed)
        {
          return {graph.get_weight(std::make_pair(edge.second, edge.first))};
        }
      else if constexpr (Parallel_Edges)
        {
          return graph.get_weight(edge);
        }
      else
        {
          return {graph.get_weight(edge)};
        }
    }

    [[nodiscard]] node_id_type
    size() const
    {
      return graph.size();
    };

    [[nodiscard]] bool
    empty() const
    {
      return graph.empty();
    };

    [[nodiscard]] std::set<node_id_type> const &
    get_output_nodes(node_id_type const &id) const
    {
      if constexpr (t_Reversed)
        {
          return graph.get_input_nodes(id);
        }
      else
        {
          return graph.get_output_nodes(id);
        }
    };


    Node_Type const &
    operator[](node_id_type const &id) const
    {
      return graph[id];
    };

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    cbegin() const
    {
      return graph.cbegin();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    cend() const
    {
      return graph.cend();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    begin() const
    {
      return cbegin();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    end() const
    {
      return cend();
    }


    Weighted_Graph<Graph_Type, !t_Reversed, Node_Type, Node_Collection_Type, Weight_Type>
    reverse() const
    {
      return Weighted_Graph<Graph_Type, !t_Reversed, Node_Type, Node_Collection_Type, Weight_Type>(graph);
    }


    explicit Weighted_Graph(Graph_Type const &g)
      : base_Weighted_Graph()
      , graph(g)
    {}

    ~Weighted_Graph() override = default;

  private:
    Graph_Type const &graph;
  };
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_WGRAPH_H
