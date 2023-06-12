#ifndef NETWORK_BUTCHER_WEIGHTED_GRAPH_H
#define NETWORK_BUTCHER_WEIGHTED_GRAPH_H

#include <network_butcher/Traits/traits.h>


namespace network_butcher::kfinder
{
  template <typename InternalGraphType,
            bool t_Reversed                 = false,
            typename t_Node_Type            = typename InternalGraphType::Node_Type,
            typename t_Node_Collection_Type = typename InternalGraphType::Node_Collection_Type,
            typename t_Weight_Type          = Time_Type>
  class Weighted_Graph;

  /// Helper Weighted graph class. It is used to identify Weighted_Graph independently of its template parameters
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

  /// Proxy class for the specified graph
  /// \tparam InternalGraphType The type of graph to proxy
  /// \tparam t_Reversed If the resulting graph is reversed
  /// \tparam t_Node_Type The type of node of the internal graph
  /// \tparam t_Node_Collection_Type The type of collection of nodes of the internal graph
  /// \tparam t_Weight_Type The type of weight of the internal graph
  template <typename InternalGraphType,
            bool t_Reversed,
            typename t_Node_Type,
            typename t_Node_Collection_Type,
            typename t_Weight_Type>
  class Weighted_Graph : Base_Weighted_Graph
  {
  public:
    /// Alias for the weight type
    using Weight_Type = t_Weight_Type;

    /// Alias for the edge type
    using Edge_Type = std::pair<Node_Id_Type, Node_Id_Type>;

    /// Alias for the graph type
    using Graph_Type = InternalGraphType;

    /// Alias for the weights of a pair of nodes
    using Weight_Edge_Type = std::multiset<Weight_Type>;

    /// Alias for the node type
    using Node_Type = t_Node_Type;

    /// Alias for the node collection type
    using Node_Collection_Type = t_Node_Collection_Type;


    /// It should return the weight(s) of the edge
    /// \param edge The edge
    /// \return The weight(s) of the edge
    [[nodiscard]] auto
    get_weight(Edge_Type const &edge) const -> Weight_Edge_Type;

    /// It should return the number of nodes
    /// \return The number of nodes
    [[nodiscard]] auto
    size() const -> Node_Id_Type;

    /// It should return whether the graph is empty
    /// \return Whether the graph is empty
    [[nodiscard]] auto
    empty() const -> bool;

    /// It should return the out neighbours of the node with id in the graph
    /// \param id The ids of the neighbours
    [[nodiscard]] auto
    get_output_nodes(Node_Id_Type const &id) const -> std::set<Node_Id_Type> const &;

    /// It should return the node (as a constant reference) with the specified id
    /// \param id The id of the node
    auto
    operator[](Node_Id_Type const &id) const -> Node_Type const &;

    auto
    cbegin() const -> typename Node_Collection_Type::const_iterator;

    auto
    cend() const -> typename Node_Collection_Type::const_iterator;

    auto
    begin() const -> typename Node_Collection_Type::const_iterator
    {
      return cbegin();
    }

    auto
    end() const -> typename Node_Collection_Type::const_iterator
    {
      return cend();
    }

    /// It should return the reversed graph, that is the graph with the direction of the edges reversed
    /// \return The reversed graph
    [[nodiscard]] auto
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

  /// Simple concept that checks if T is a child class of Base_Weighted_Graph
  /// \tparam T The considered type. It should be a child of Base_Weighted_Graph
  template <typename T>
  concept Valid_Weighted_Graph = std::is_base_of_v<Base_Weighted_Graph, T>;
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_WEIGHTED_GRAPH_H