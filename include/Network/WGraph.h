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
    template <bool Parallel_Edges, typename t_Node_Type = Node>
    class WGraph : public MWGraph<Parallel_Edges, t_Node_Type>
    {
    private:
      using Parent_type = MWGraph<Parallel_Edges, t_Node_Type>;

    public:
      using Dependencies_Type    = Parent_type::Dependencies_Type;
      using Node_Type            = Parent_type::Node_Type;
      using Node_Collection_Type = Parent_type::Node_Collection_Type;

      using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<weight_type>, weight_type>;


      template <typename A, typename B>
      explicit WGraph(A &&v, B &&dep)
        : Parent_type(1, std::forward<A>(v), std::forward<B>(dep))
      {}


      /// Get the weight for the given edge
      /// \param edge The edge
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
      set_weight(edge_type const &edge, weight_type weight)
      {
        Parent_type::set_weight(0, edge, weight);
      }

      /// Sets the weight for the given edge on the given device
      /// \param device The device id
      /// \param edge The edge
      /// \param weight The weight
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

      using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<weight_type>, weight_type>;


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
      set_weight(edge_type const &edge, weight_type weight)
      {
        Parent_type::set_weight(0, edge, weight);
      }

      /// Sets the weight for the given edge on the given device
      /// \param device The device id
      /// \param edge The edge
      /// \param weight The weight
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
  } // namespace types

  namespace kfinder
  {

    /// (Partial) Specialization of Weighted_Graph
    /// \tparam T The internal type of WGraph
    template <bool Parallel_Edges, typename t_Node_Type, bool Reversed>
    class Weighted_Graph<network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>,
                         typename network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>::Node_Type,
                         typename network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>::Node_Collection_Type,
                         typename network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>::Dependencies_Type,
                         Reversed>
    {
    public:
      using Node_Id_Type         = std::size_t;
      using Edge_Type            = std::pair<Node_Id_Type, Node_Id_Type>;
      using Graph_Type           = typename network_butcher::types::WGraph<Parallel_Edges, t_Node_Type>;
      using Node_Type            = Graph_Type::Node_Type;
      using Dependencies_Type    = Graph_Type::Dependencies_Type;
      using Node_Collection_Type = std::vector<Node_Type>;
      using Weight_Edge_Type     = std::multiset<weight_type>;


      [[nodiscard]] Weight_Edge_Type
      get_weight(Edge_Type const &edge) const
      {
        if constexpr (Reversed && Parallel_Edges)
          {
            return graph.get_weight(std::make_pair(edge.second, edge.first));
          }
        else if constexpr (Reversed)
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

      [[nodiscard]] std::size_t
      size() const
      {
        return graph.size();
      };

      [[nodiscard]] bool
      empty() const
      {
        return graph.empty();
      };

      [[nodiscard]] std::set<Node_Id_Type> const &
      get_output_nodes(Node_Id_Type const &id) const
      {
        if constexpr (Reversed)
          {
            return reversed_neighboors[id].second;
          }
        else
          {
            return graph.get_output_nodes(id);
          }
      };


      Node_Type const &
      operator[](Node_Id_Type const &id) const
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


      explicit Weighted_Graph(Graph_Type const &g)
        : graph(g)
      {
        if constexpr (Reversed)
          {
            reversed_neighboors.resize(size());

            for (auto const &tail_node : *this)
              {
                auto const &tail = tail_node.get_id();
                for (auto const &head : g.get_output_nodes(tail))
                  {
                    reversed_neighboors[head].second.insert(tail);
                    reversed_neighboors[tail].first.insert(head);
                  }
              }
          }
      }

      Weighted_Graph(Weighted_Graph const &) = default;

    private:
      Graph_Type const                                     &graph;
      std::conditional_t<Reversed, Dependencies_Type, bool> reversed_neighboors;
    };
  } // namespace kfinder

} // namespace network_butcher

#endif // NETWORK_BUTCHER_WGRAPH_H
