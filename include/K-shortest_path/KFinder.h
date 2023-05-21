//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KFINDER_H
#define NETWORK_BUTCHER_KFINDER_H

#include "KFinder_Base_traits.h"

namespace network_butcher::kfinder
{

  /// A (pure) virtual class to find the K shortest path for a given graph
  /// \tparam Graph_type The graph type. To be able to use the class, you must specialize Weighted_Graph<Graph_type>
  template <class Graph_type,
            bool                 Only_Distance                  = false,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<Graph_type>>
  class KFinder
  {
  protected:
    using Weighted_Graph_Complete_Type = t_Weighted_Graph_Complete_Type;
    using Weight_Type                  = Weighted_Graph_Complete_Type::Weight_Type;


    Weighted_Graph_Complete_Type graph;
    node_id_type                 root;
    node_id_type                 sink;

  public:
    /// Applies a K-shortest path algorithm to find the k-shortest paths on the given graph (from the first node to
    /// the last one)
    /// \param K The number of shortest paths to find
    /// \return The shortest paths
    [[nodiscard]] virtual std::
      conditional_t<Only_Distance, std::vector<Weight_Type>, std::vector<t_path_info<Weight_Type>>>
      compute(std::size_t K) const = 0;


    explicit KFinder(Graph_type const &g, node_id_type root, node_id_type sink)
      : graph(g)
      , root(root)
      , sink(sink){};


    explicit KFinder(Weighted_Graph_Complete_Type const &g, node_id_type root, node_id_type sink)
      : graph(g)
      , root(root)
      , sink(sink){};

    virtual ~KFinder() = default;
  };

} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_KFINDER_H
