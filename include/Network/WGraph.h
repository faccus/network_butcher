//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_WGRAPH_H
#define NETWORK_BUTCHER_WGRAPH_H

#include "MWGraph.h"
#include "Weighted_Graph.h"


namespace network_butcher
{
  namespace types
  {
    /// Just another weighted graph class...
    /// \tparam T Type of the content of the nodes
    template <bool Parallel_Edges, typename t_Node_Type = Node, typename t_weight_type = weight_type>
    class WGraph : public MWGraph<Parallel_Edges, t_Node_Type, t_weight_type>
    {
    private:
      using Parent_type = MWGraph<Parallel_Edges, t_Node_Type, t_weight_type>;

    public:
      using Dependencies_Type    = Parent_type::Dependencies_Type;
      using Node_Type            = Parent_type::Node_Type;
      using Node_Collection_Type = Parent_type::Node_Collection_Type;

      using Weight_Type = Parent_type::Weight_Type;

      using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<Weight_Type>, Weight_Type>;


      template <typename A, typename B>
      explicit WGraph(A &&v, B &&dep)
        : Parent_type(1, std::forward<A>(v), std::forward<B>(dep))
      {}


      /// Get the weight for the given edge
      /// \param edge The edge
      /// \param print_missing Print an error message if the weights is missing
      /// \return The weight
      [[nodiscard]] Edge_Weight_Type
      get_weight(edge_type const &edge, bool print_missing = true) const
      {
        if (print_missing && !check_weight(edge))
          {
            std::cout << "Requested a non-existing weight for edge (" << edge.first << ", " << edge.second
                      << "). I will return .0" << std::endl;
          }

        return Parent_type::get_weight(0, edge);
      }

      /// Set the weight for the given edge
      /// \param edge The edge
      /// \param weight The weight
      void
      set_weight(edge_type const &edge, Weight_Type const &weight)
      {
        Parent_type::set_weight(0, edge, weight);
      }

      /// Sets the weight for the given edge on the given device
      /// \param edge The edge
      /// \param weights The weight
      template <bool cond = Parallel_Edges, std::enable_if_t<cond, bool> = true>
      void
      set_weight(edge_type const &edge, Edge_Weight_Type const &weights)
        requires cond
      {
        Parent_type::set_weight(0, edge, weights);
      }


      [[nodiscard]] bool
      check_weight(edge_type const &edge) const
      {
        return Parent_type::check_weight(0, edge);
      }

      ~WGraph() override = default;
    };

    /// Just another weighted graph class...
    /// \tparam T Type of the content of the nodes
    template <bool Parallel_Edges, typename T>
    class WGraph<Parallel_Edges, CNode<Content<T>>> : public MWGraph<Parallel_Edges, CNode<Content<T>>>
    {
    private:
      using Parent_type = MWGraph<Parallel_Edges, CNode<Content<T>>>;

    public:
      using Dependencies_Type    = Parent_type::Dependencies_Type;
      using Node_Type            = Parent_type::Node_Type;
      using Node_Collection_Type = Parent_type::Node_Collection_Type;

      using Weight_Type = Parent_type::Weight_Type;

      using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<Weight_Type>, Weight_Type>;


      template <typename A, typename B>
      explicit WGraph(A &&v, B &&dep)
        : Parent_type(1, std::forward<A>(v), std::forward<B>(dep))
      {}


      template <typename A>
      explicit WGraph(A &&v)
        : Parent_type(1, std::forward<A>(v))
      {}


      /// Get the weight for the given edge
      /// \param edge The edge
      /// \param print_missing Print an error message if the weights is missing
      /// \return The weight
      [[nodiscard]] Edge_Weight_Type
      get_weight(edge_type const &edge, bool print_missing = true) const
      {
        if (print_missing && !check_weight(edge))
          {
            std::cout << "Requested a non-existing weight for edge (" << edge.first << ", " << edge.second
                      << "). I will return .0" << std::endl;
          }

        return Parent_type::get_weight(0, edge);
      }

      /// Set the weight for the given edge
      /// \param edge The edge
      /// \param weight The weight
      void
      set_weight(edge_type const &edge, Weight_Type const &weight)
      {
        Parent_type::set_weight(0, edge, weight);
      }

      /// Sets the weight for the given edge
      /// \param edge The edge
      /// \param weights The weights
      template <bool cond = Parallel_Edges, std::enable_if_t<cond, bool> = true>
      void
      set_weight(edge_type const &edge, Edge_Weight_Type const &weights)
        requires cond
      {
        Parent_type::set_weight(0, edge, weights);
      }

      /// It checks if the given edge exists
      /// \param edge The edge
      /// \return True if it exists, false otherwise
      [[nodiscard]] bool
      check_weight(edge_type const &edge) const
      {
        return Parent_type::check_weight(0, edge);
      }

      ~WGraph() override = default;
    };
  } // namespace types

  namespace kfinder
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
  } // namespace kfinder

} // namespace network_butcher

#endif // NETWORK_BUTCHER_WGRAPH_H
