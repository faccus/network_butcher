//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_WGRAPH_H
#define NETWORK_BUTCHER_WGRAPH_H

#include "mwgraph.h"
#include "weighted_graph.h"


namespace network_butcher::types
{
  /// Just another weighted graph class...
  /// \tparam Parallel_Edges If true, the graph will support parallel edges
  /// \tparam Template_Node_Type Type of nodes
  /// \tparam t_weight_type Type of the weight
  template <bool Parallel_Edges, typename Template_Node_Type = Node, typename t_weight_type = Time_Type>
  class WGraph : public MWGraph<Parallel_Edges, Template_Node_Type, t_weight_type>
  {
  private:
    using Parent_Type = MWGraph<Parallel_Edges, Template_Node_Type, t_weight_type>;

    using Parent_Type::check_weight;
    using Parent_Type::get_num_devices;
    using Parent_Type::get_weight;
    using Parent_Type::print_graph;
    using Parent_Type::set_weight;

  public:
    using Dependencies_Type    = Parent_Type::Dependencies_Type;
    using Node_Type            = Parent_Type::Node_Type;
    using Node_Collection_Type = Parent_Type::Node_Collection_Type;

    using Weight_Type            = Parent_Type::Weight_Type;
    using Edge_Weight_Type       = Parent_Type::Edge_Weight_Type;
    using Weight_Collection_Type = Parent_Type::Weight_Collection_Type;

    template <typename A, typename B>
    explicit WGraph(A &&v, B &&dep)
      : Parent_Type(1, std::forward<A>(v), std::forward<B>(dep))
    {}


    /// Checks if the given edge has a weight
    /// \param device The device id
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] auto
    check_weight(Edge_Type const &edge) const
    {
      return Parent_Type::check_weight(0, edge);
    }


    /// Get the weight for the given edge
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] auto
    get_weight(Edge_Type const &edge) const
    {
      return Parent_Type::get_weight(0, edge);
    }


    /// Sets the weight for the given edge
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(Edge_Type const &edge, t_weight_type const &weight)
    {
      Parent_Type::set_weight(0, edge, weight);
    }


    /// Sets the weight for the given edge
    /// \param edge The edge
    /// \param weights The weight
    void
    set_weight(Edge_Type const &edge, Edge_Weight_Type weights)
      requires Parallel_Edges
    {
      Parent_Type::set_weight(0, edge, weights);
    }


    /// Simple helper function that will print the graph
    /// \return The graph description
    [[nodiscard]] auto
    print_graph() const -> std::string override
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
  class WGraph<Parallel_Edges, CNode<Content<T>>> : public MWGraph<Parallel_Edges, CNode<Content<T>>>
  {
  private:
    using Parent_Type   = MWGraph<Parallel_Edges, CNode<Content<T>>>;
    using t_Node_Type   = Parent_Type::Node_Type;
    using t_weight_type = Parent_Type::Weight_Type;

    using Parent_Type::check_weight;
    using Parent_Type::get_num_devices;
    using Parent_Type::get_weight;
    using Parent_Type::print_graph;
    using Parent_Type::set_weight;

  public:
    using Dependencies_Type    = Parent_Type::Dependencies_Type;
    using Node_Type            = Parent_Type::Node_Type;
    using Node_Collection_Type = Parent_Type::Node_Collection_Type;

    using Weight_Type            = Parent_Type::Weight_Type;
    using Edge_Weight_Type       = Parent_Type::Edge_Weight_Type;
    using Weight_Collection_Type = Parent_Type::Weight_Collection_Type;

    template <typename A, typename B>
    explicit WGraph(A &&v, B &&dep)
      : Parent_Type(1, std::forward<A>(v), std::forward<B>(dep))
    {}

    explicit WGraph(Node_Collection_Type const &v)
      : Parent_Type(1, v)
    {}
    explicit WGraph(Node_Collection_Type &&v)
      : Parent_Type(1, std::move(v))
    {}


    /// Checks if the given edge has a weight
    /// \param device The device id
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] auto
    check_weight(Edge_Type const &edge) const
    {
      return Parent_Type::check_weight(0, edge);
    }


    /// Get the weight for the given edge
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] auto
    get_weight(Edge_Type const &edge) const
    {
      return Parent_Type::get_weight(0, edge);
    }


    /// Sets the weight for the given edge
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(Edge_Type const &edge, t_weight_type const &weight)
    {
      Parent_Type::set_weight(0, edge, weight);
    }


    /// Sets the weight for the given edge
    /// \param edge The edge
    /// \param weights The weight
    void
    set_weight(Edge_Type const &edge, Edge_Weight_Type weights)
      requires Parallel_Edges
    {
      Parent_Type::set_weight(0, edge, weights);
    }


    /// Simple helper function that will print the graph
    /// \return The graph description
    [[nodiscard]] auto
    print_graph() const -> std::string override
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
                       Time_Type> : Base_Weighted_Graph
  {
  public:
    using Weight_Type = Time_Type;

    using Edge_Type = std::pair<Node_Id_Type, Node_Id_Type>;

    using Graph_Type       = network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>;
    using Weight_Edge_Type = std::multiset<Weight_Type>;

    using Node_Type            = t_Node_Type;
    using Node_Collection_Type = std::vector<Node_Type>;


    [[nodiscard]] auto
    get_weight(Edge_Type const &edge) const -> Weight_Edge_Type
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

    [[nodiscard]] auto
    size() const -> std::size_t
    {
      return graph.size();
    };

    [[nodiscard]] auto
    empty() const -> bool
    {
      return graph.empty();
    };

    [[nodiscard]] auto
    get_output_nodes(Node_Id_Type const &id) const -> std::set<Node_Id_Type> const &
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


    auto
    operator[](Node_Id_Type const &id) const -> Node_Type const &
    {
      return graph[id];
    };

    [[nodiscard]] auto
    cbegin() const -> typename Node_Collection_Type::const_iterator
    {
      return graph.cbegin();
    }

    [[nodiscard]] auto
    cend() const -> typename Node_Collection_Type::const_iterator
    {
      return graph.cend();
    }

    [[nodiscard]] auto
    begin() const -> typename Node_Collection_Type::const_iterator
    {
      return cbegin();
    }

    [[nodiscard]] auto
    end() const -> typename Node_Collection_Type::const_iterator
    {
      return cend();
    }


    auto
    reverse() const -> Weighted_Graph<Graph_Type, !t_Reversed, Node_Type, Node_Collection_Type, Weight_Type>
    {
      return Weighted_Graph<Graph_Type, !t_Reversed, Node_Type, Node_Collection_Type, Weight_Type>(graph);
    }


    explicit Weighted_Graph(Graph_Type const &g)
      : Base_Weighted_Graph()
      , graph(g)
    {}

    ~Weighted_Graph() override = default;

  private:
    Graph_Type const &graph;
  };
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_WGRAPH_H
