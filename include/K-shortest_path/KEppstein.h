//
// Created by faccus on 01/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_H
#define NETWORK_BUTCHER_KEPPSTEIN_H

#include "Heap_traits.h"
#include "KFinder.h"

namespace network_butcher_kfinder
{
  /// This class implements the Eppstein K-shortest path algorithm
  /// \tparam Graph_type The graph type
  template <class Graph_type>
  class KFinder_Eppstein : public KFinder<Graph_type>
  {
  private:
    using base          = KFinder<Graph_type>;


    /// Given the successors collection and the sidetrack distances, it will
    /// construct the h_out map
    /// \param successors The list of the successors of every node (the node
    /// following the current one in the shortest path)
    /// \param sidetrack_distances The collection of the sidetrack distances for
    /// all the sidetrack edges
    /// \return H_out map
    [[nodiscard]] H_out_collection
    construct_h_out(std::vector<node_id_type> const &successors,
                    weights_collection_type const   &sidetrack_distances) const;


    /// It will produce the map associating every node to its corresponding H_g
    /// map \param h_out The collection of h_outs \param successors The
    /// successors list \return The map associating every node to its
    /// corresponding H_g map
    [[nodiscard]] H_g_collection
    construct_h_g(H_out_collection const &h_out, std::vector<node_id_type> const &successors) const;


    /// The basic function for the Eppstein algorithm
    /// \param weights The weights of the edges
    /// \param K The number of shortest paths
    /// \param dij_res The result of dijkstra
    /// \return The (implicit) k shortest paths
    [[nodiscard]] std::vector<implicit_path_info>
    basic_eppstein(std::size_t K, dijkstra_result_type const &dij_res) const;

  public:
    /// Applies the Eppstein algorithm to find the k-shortest paths on the given
    /// graph (from the first node to the last one)
    /// \param K The number of shortest paths to find
    /// \return The shortest paths
    [[nodiscard]] std::vector<path_info>
    compute(std::size_t K) const override;

    explicit KFinder_Eppstein(Graph_type const &g)
      : base(g){};

    ~KFinder_Eppstein() override = default;
  };


  template <class Graph_type>
  H_out_collection
  KFinder_Eppstein<Graph_type>::construct_h_out(const std::vector<node_id_type> &successors,
                                                const weights_collection_type   &sidetrack_distances) const
  {
    H_out_collection h_out;
    auto const      &graph = base::graph;

    for (auto const &tail_node : graph.get_nodes())
      {
        auto h_out_entry_it = h_out.insert(h_out.cend(), {tail_node.get_id(), std::make_shared<H_out<edge_info>>()});
        h_out_entry_it->second->heap.id = tail_node.get_id();

        auto const &tail = tail_node.get_id();
        for (auto const &head : graph.get_neighbors()[tail].second)
          {
            if (head != successors[tail])
              {
                auto sidetrack_it = sidetrack_distances.find({tail, head});
                if (sidetrack_it != sidetrack_distances.cend())
                  {
                    auto &children = h_out_entry_it->second->heap.children;

                    edge_info tmp(sidetrack_it->first, sidetrack_it->second);

                    children.insert(std::move(tmp));
                  }
              }
          }
      }

    return h_out;
  }

  template <class Graph_type>
  H_g_collection
  KFinder_Eppstein<Graph_type>::construct_h_g(const H_out_collection          &h_out,
                                                     const std::vector<node_id_type> &successors) const // O(N*log(N))
  {
    H_g_collection h_g;

    auto const &graph = base::graph;
    auto const &nodes = graph.get_nodes();
    auto const &num_nodes = nodes.size();

    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(num_nodes);

    // Prepare the H_g map
    for (auto i = 0; i < num_nodes; ++i)
      {
        auto it       = h_g.insert(h_g.cend(), {i, H_g()});
        it->second.id = i;
      }

    // sp_dependencies contains for every node its predecessors (the nodes that along the shortest path to the sink have
    // as a successor the node itself). Notice that the sum of the sizes of all the stored sets is at most N since two
    // nodes cannot share the same predecessor along the shortest path
    for (auto i = 0; i < num_nodes; ++i) // O(N)
      {
        auto const &tmp = successors[i];

        if (tmp != i)
          sp_dependencies[tmp].insert(i); // O(log(N))
      }

    auto const sink = nodes.size() - 1;

    auto iterator = h_out.find(sink); // O(1)

    // Prepare the last H_g
    if (iterator != h_out.cend() && !iterator->second->heap.children.empty())
      {
        h_g[sink].children.emplace(iterator->second); // O(log(N))
      }

    std::queue<node_id_type> queue;

    for (auto i = 0; i < h_g.size(); ++i) // O(N)
      if (successors[i] == sink)
        queue.push(i);

    // We loop though the queue in such a way that the H_g of the successor of every node in the queue itself has been
    // already computed
    while (!queue.empty()) // O(N)
      {
        auto const &front_element = queue.front();

        auto &deps = sp_dependencies[front_element]; // O(1)

        auto &heap_node = h_g[front_element]; // O(log(N))

        iterator = h_out.find(front_element);

        // Add H_out and...
        if (iterator != h_out.cend() && !iterator->second->heap.children.empty())
          heap_node.children.emplace(iterator->second); // O(1)

        auto const &tmo = h_g[successors[front_element]];

        // the H_g of the successor along the shortest path
        if (!tmo.children.empty())
          heap_node.children.insert(tmo.children.begin(),
                                    tmo.children.end()); // O(1)

        // Among the different iterations, this loop is performed at most N times. Moreover, every iteration of the loop
        // will add a new node to the queue. This means that this loop doesn't change the worst case complexity of the
        // overall method
        for (auto &n : deps)
          queue.push(n);

        queue.pop(); // O(1)
      }

    return h_g;
  }

  template <class Graph_type>
  std::vector<implicit_path_info>
  KFinder_Eppstein<Graph_type>::basic_eppstein(std::size_t K, const dijkstra_result_type &dij_res) const
  {
    auto const &graph = base::graph;

    auto const sidetrack_distances_res = base::sidetrack_distances(dij_res.second);              // O(E)
    auto const shortest_path           = Shortest_path_finder::shortest_path_finder(graph, dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto h_out = construct_h_out(successors, sidetrack_distances_res); // O(N+E)
    auto h_g = construct_h_g(h_out, successors); // O(N*log(N))

    return base::general_algo_eppstein(K, dij_res, sidetrack_distances_res, h_g, h_out);
  }

  template <class Graph_type>
  std::vector<path_info>
  KFinder_Eppstein<Graph_type>::compute(std::size_t K) const
  {
    auto const &graph = base::graph;

    if (graph.empty() || K == 0)
      return {};

    auto const dij_res = Shortest_path_finder::shortest_path_tree(graph); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {Shortest_path_finder::shortest_path_finder(graph, dij_res, 0)};


    auto const epp_res = basic_eppstein(K, dij_res);

    return base::helper_eppstein(dij_res, epp_res);
  }

} // namespace network_butcher_kfinder


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
