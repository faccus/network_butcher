//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KFINDER_H
#define NETWORK_BUTCHER_KFINDER_H

namespace network_butcher::kfinder
{
  /// A (pure) virtual class to find the K shortest path for a given graph
  /// \tparam Graph_type The graph type. To be able to use the class, you must specialize Weighted_Graph<Graph_type>
  template <class Graph_type>
  class KFinder
  {
  protected:
    Weighted_Graph<Graph_type> graph;

  public:
    /// Applies a K-shortest path algorithm to find the k-shortest paths on the given graph (from the first node to
    /// the last one)
    /// \param K The number of shortest paths to find
    /// \return The shortest paths
    [[nodiscard]] virtual std::vector<path_info>
    compute(std::size_t K) const = 0;

    explicit KFinder(Graph_type const &g)
      : graph(g){};

    explicit KFinder(Weighted_Graph<Graph_type> const &g)
      : graph(g){};

    KFinder(KFinder const &)     = default;
    KFinder(KFinder &&) noexcept = default;

    KFinder &
    operator=(KFinder const &) = default;
    KFinder &
    operator=(KFinder &&) noexcept = default;

    virtual ~KFinder() = default;
  };

} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_KFINDER_H
