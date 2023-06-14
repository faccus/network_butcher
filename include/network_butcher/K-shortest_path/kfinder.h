#ifndef NETWORK_BUTCHER_KFINDER_H
#define NETWORK_BUTCHER_KFINDER_H

#include <network_butcher/Traits/traits.h>
#include <network_butcher/K-shortest_path/weighted_graph.h>

namespace network_butcher::kfinder
{

  /// A (pure) virtual class to find the K shortest path for a given graph
  /// \tparam GraphType The graph type. To be able to use the class, you must specialize Weighted_Graph<Graph_type>
  template <class GraphType,
            bool                 Only_Distance                  = false,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<GraphType>>
  class KFinder
  {
  protected:
    using Weighted_Graph_Complete_Type = t_Weighted_Graph_Complete_Type;
    using Weight_Type                  = Weighted_Graph_Complete_Type::Weight_Type;

    Weighted_Graph_Complete_Type graph;
    Node_Id_Type                 root;
    Node_Id_Type                 sink;

  public:
    using Output_Type =
      std::conditional_t<Only_Distance, std::vector<Weight_Type>, std::vector<Templated_Path_Info<Weight_Type>>>;


    /// Applies a K-shortest path algorithm to find the k-shortest paths on the given graph (from the first node to
    /// the last one)
    /// \param K The number of shortest paths to find
    /// \return The shortest paths
    [[nodiscard]] virtual auto
    compute(std::size_t K) const -> Output_Type = 0;


    /// Constructor for a K shortest path class
    /// \param g The graph
    /// \param root The root node id
    /// \param sink The sink node id
    explicit KFinder(GraphType const &g, Node_Id_Type root, Node_Id_Type sink)
      : graph(g)
      , root(root)
      , sink(sink){};


    /// Constructor for a K shortest path class
    /// \param g The graph
    /// \param root The root node id
    /// \param sink The sink node id
    explicit KFinder(Weighted_Graph_Complete_Type const &g, Node_Id_Type root, Node_Id_Type sink)
      : graph(g)
      , root(root)
      , sink(sink){};

    virtual ~KFinder() = default;
  };

} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_KFINDER_H