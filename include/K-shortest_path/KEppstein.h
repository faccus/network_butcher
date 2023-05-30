//
// Created by faccus on 01/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_H
#define NETWORK_BUTCHER_KEPPSTEIN_H

#include <list>

#include "Heap_traits.h"
#include "basic_KEppstein.h"

namespace network_butcher::kfinder
{
  /// This class implements the Eppstein K-shortest path algorithm
  /// \tparam Graph_type The graph type
  template <typename Graph_type,
            bool                 Only_Distance                  = false,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<Graph_type>>
  class KFinder_Eppstein final : public basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>
  {
  private:
    using base = basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>;

  public:
    using Output_Type = typename base::Output_Type;

  private:
    using Weight_Type = base::Weight_Type;

    using edge_info = base::edge_info;

    using internal_weight_collection = base::internal_weight_collection;
    using dijkstra_result_type       = base::dijkstra_result_type;

    using H_g_collection   = base::H_g_collection;
    using H_out_collection = base::H_out_collection;


    /// Given the successors collection and the sidetrack distances, it will construct the h_out map
    /// \param successors The list of the successors of every node (the node following the current one in the
    /// shortest path)
    /// \param sidetrack_distances The collection of the sidetrack distances for all the sidetrack edges
    /// \return H_out map
    [[nodiscard]] H_out_collection
    construct_h_out(std::vector<node_id_type> const  &successors,
                    internal_weight_collection const &sidetrack_distances) const;


    /// It will produce the map associating every node to its corresponding H_g map
    /// \param h_out_collection The collection of h_outs
    /// \param successors The successors list
    /// \return The map associating every node to its corresponding H_g map
    [[nodiscard]] H_g_collection
    construct_h_g(H_out_collection const &h_out_collection, std::vector<node_id_type> const &successors) const;


    /// The basic function for the Eppstein algorithm
    /// \param K The number of shortest paths
    /// \param dij_res The result of dijkstra
    /// \return The (implicit) k shortest paths
    [[nodiscard]] Output_Type
    start(std::size_t K, dijkstra_result_type const &dij_res) const override;

  public:
    explicit KFinder_Eppstein(Graph_type const &g, std::size_t root, std::size_t sink)
      : base(g, root, sink){};

    explicit KFinder_Eppstein(Weighted_Graph<Graph_type> const &g, std::size_t root, std::size_t sink)
      : base(g, root, sink){};

    ~KFinder_Eppstein() override = default;
  };


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::Output_Type
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::start(
    std::size_t                                   K,
    const KFinder_Eppstein::dijkstra_result_type &dij_res) const
  {
    auto const  sidetrack_distances_res = base::sidetrack_distances(dij_res); // O(E)
    auto const &successors              = dij_res.first;

    auto h_out = construct_h_out(successors, sidetrack_distances_res); // O(N+E)
    auto h_g   = construct_h_g(h_out, successors);                     // O(N*log(N))

    return base::general_algo_eppstein(K, dij_res, sidetrack_distances_res, h_g, h_out);
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::H_out_collection
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_h_out(
    const std::vector<node_id_type>  &successors,
    const internal_weight_collection &sidetrack_distances) const
  {
    H_out_collection h_out_collection;
    auto const      &graph = base::graph;

    for (auto const &tail_node : graph)
      {
        auto const &tail           = tail_node.get_id();
        auto const &tail_successor = successors[tail];
        if (tail_successor == std::numeric_limits<node_id_type>::max())
          continue;

        auto &h_out =
          h_out_collection.insert(h_out_collection.cend(), {tail, typename H_out_collection::mapped_type(tail)})
            ->second;

        // The output neighbors of the current node
        auto const &head_nodes = graph.get_output_nodes(tail);

        // Loop through the output neighbors of the current node
        for (auto const &head : head_nodes)
          {
            auto [begin, end] = sidetrack_distances.equal_range(edge_type{tail, head});

            for (; begin != end; ++begin)
              {
                h_out.push(edge_info{begin->first, begin->second});
              }
          }
      }

    return h_out_collection;
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::H_g_collection
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_h_g(
    const H_out_collection          &h_out_collection,
    const std::vector<node_id_type> &successors) const // O(N*log(N))
  {
    H_g_collection h_g_collection;

    auto const &graph     = base::graph;
    auto const &num_nodes = graph.size();
    auto const &sink      = base::sink;

    // sp_dependencies contains the predecessors of every node in the shortest path. Notice
    // that the sum of the sizes of all the stored sets is at most N
    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(num_nodes);

    for (auto const &node : graph)
      {
        auto const &node_id   = node.get_id();
        auto const &successor = successors[node_id];

        if (successor == std::numeric_limits<node_id_type>::max())
          continue;

        // Prepare the H_g map
        auto it =
          h_g_collection.emplace_hint(h_g_collection.cend(), node_id, typename H_g_collection::mapped_type(node_id));

        if (sink != node_id)
          sp_dependencies[successor].insert(node_id); // O(log(N))
      }

    // The actual generation of the H_g should now start from the sink node
    auto h_out_iterator = h_out_collection.find(sink); // O(1)

    // Prepare the last H_g
    if (h_out_iterator != h_out_collection.cend() && !h_out_iterator->second.empty())
      {
        h_g_collection.find(sink)->second.push(h_out_iterator); // O(log(N))
      }

    // Now, we have to find the nodes whose successor is the sink itself
    std::queue<node_id_type> queue;
    for (auto const &node : graph) // O(N)
      {
        auto const &id = node.get_id();
        if (successors[id] == sink && id != sink) // O(log(N))
          queue.push(id);
      }

    // Loop through the queue
    while (!queue.empty()) // O(N)
      {
        auto const &front_element = queue.front();

        // Find the "new" nodes that must be added to the queue. At the end this iteration, their H_g can be computed
        auto       &deps          = sp_dependencies[front_element];                         // O(1)
        auto       &h_g           = h_g_collection.find(front_element)->second;             // O(1)
        auto const &successor_h_g = h_g_collection.find(successors[front_element])->second; // O(1)

        // the H_g of the successor along the shortest path
        if (!successor_h_g.empty())
          h_g.overwrite_children(successor_h_g); // O(1)

        h_out_iterator = h_out_collection.find(front_element);

        // Add H_out and...
        if (h_out_iterator != h_out_collection.cend() && !h_out_iterator->second.empty())
          h_g.push(h_out_iterator); // O(1)


        // Among the different iterations, this loop is performed at most N times. Moreover, every iteration of the
        // loop will add a new node to the queue. This means that this loop doesn't change the worst case complexity
        // of the overall method
        for (auto &n : deps)
          queue.push(n);

        queue.pop(); // O(1)
      }

    return h_g_collection;
  }
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
