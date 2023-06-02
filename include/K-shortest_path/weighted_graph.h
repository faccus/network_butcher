//
// Created by faccus on 2/19/23.
//

#ifndef NETWORK_BUTCHER_WEIGHTED_GRAPH_H
#define NETWORK_BUTCHER_WEIGHTED_GRAPH_H

#include "basic_traits.h"


namespace network_butcher::kfinder
{
  class Base_Weighted_Graph;

  template <typename Base_Weighted_Graph,
            bool t_Reversed                 = false,
            typename t_Node_Type            = typename Base_Weighted_Graph::Node_Type,
            typename t_Node_Collection_Type = typename Base_Weighted_Graph::Node_Collection_Type,
            typename t_Weight_Type          = Time_Type>
  class Weighted_Graph;

  class Base_Weighted_Graph
  {
  private:
    Base_Weighted_Graph() = default;

    template <typename Base_Weighted_Graph,
              bool t_Reversed,
              typename t_Node_Type,
              typename t_Node_Collection_Type,
              typename t_Weight_Type>
    friend class Weighted_Graph;

  public:
    virtual ~Base_Weighted_Graph() = default;
  };

  /// Interface class for a Weighted Graph
  /// \tparam Base_Weighted_Graph The class of the weighted graph to represent
  /// \tparam Node_Type The type of a Node of the weight graph
  /// \tparam Node_Collection_Type The type of the collection containing the nodes of the graph
  template <typename Base_Weighted_Graph,
            bool t_Reversed,
            typename t_Node_Type,
            typename t_Node_Collection_Type,
            typename t_Weight_Type>
  class Weighted_Graph : Base_Weighted_Graph
  {
  public:
    using Weight_Type = t_Weight_Type;

    using Edge_Type = std::pair<Node_Id_Type, Node_Id_Type>;

    using Graph_Type       = Base_Weighted_Graph;
    using Weight_Edge_Type = std::multiset<Weight_Type>;

    using Node_Type            = t_Node_Type;
    using Node_Collection_Type = t_Node_Collection_Type;


    [[nodiscard]] Weight_Edge_Type
    get_weight(Edge_Type const &edge) const;

    [[nodiscard]] Node_Id_Type
    size() const;

    [[nodiscard]] bool
    empty() const;


    [[nodiscard]] std::set<Node_Id_Type> const &
    get_output_nodes(Node_Id_Type const &id) const;


    Node_Type const &
    operator[](Node_Id_Type const &id) const;

    typename Node_Collection_Type::const_iterator
    cbegin() const;

    typename Node_Collection_Type::const_iterator
    cend() const;

    typename Node_Collection_Type::const_iterator
    begin() const
    {
      return cbegin();
    }

    typename Node_Collection_Type::const_iterator
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
      : Base_Weighted_Graph()
      , graph(g)
    {}

    ~Weighted_Graph() override = default;

  private:
    Graph_Type const &graph;
  };
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_WEIGHTED_GRAPH_H