#ifndef NETWORK_BUTCHER_KEPPSTEIN_H
#define NETWORK_BUTCHER_KEPPSTEIN_H

#include <list>

#include "basic_keppstein.h"
#include "traits.h"
#include "weighted_graph.h"

namespace network_butcher::kfinder
{
  /// This class implements the Eppstein K-shortest path algorithm
  /// \tparam GraphType The graph type
  template <typename GraphType,
            bool                 Only_Distance                  = false,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<GraphType>>
  class KFinder_Eppstein final : public Basic_KEppstein<GraphType, Only_Distance, t_Weighted_Graph_Complete_Type>
  {
  private:
    /// The parent type. Used to access quickly to the parent methods
    using Parent_Type = Basic_KEppstein<GraphType, Only_Distance, t_Weighted_Graph_Complete_Type>;

  public:
    /// The type of the output of the algorithm
    using Output_Type = Parent_Type::Output_Type;

  private:
    /// Bring forward the graph
    using Parent_Type::graph;

    /// Bring forward the root node id
    using Parent_Type::root;

    /// Bring forward the sink node id
    using Parent_Type::sink;

    /// Type for an edge with its weight
    using Edge_Info = Parent_Type::Edge_Info;

    /// Weight Type
    using Weight_Type = Parent_Type::Weight_Type;

    /// Type of the result of the Dijkstra algorithm
    using Dijkstra_Result_Type = Parent_Type::Dijkstra_Result_Type;

    /// Type of collection of weights. Used to map edges to their sidetrack weights
    using Internal_Weight_Collection_Type = Parent_Type::Internal_Weight_Collection_Type;

    /// Type of the collection of H_g
    using H_g_collection = Parent_Type::H_g_collection;

    /// Type of the collection of H_out
    using H_out_collection = Parent_Type::H_out_collection;


    /// Given the successors collection and the sidetrack distances, it will construct the h_out map. O(M)
    /// \param successors The list of the successors of every node (the node following the current one in the
    /// shortest path)
    /// \param sidetrack_distances The collection of the sidetrack distances for all the sidetrack edges
    /// \return H_out map
    [[nodiscard]] auto
    construct_h_out(std::vector<Node_Id_Type> const       &successors,
                    Internal_Weight_Collection_Type const &sidetrack_distances) const -> H_out_collection;


    /// It will produce the map associating every node to its corresponding H_g map. O(N*log(N))
    /// \param h_out_collection The collection of h_outs
    /// \param successors The successors list
    /// \return The map associating every node to its corresponding H_g map
    [[nodiscard]] auto
    construct_h_g(H_out_collection const &h_out_collection, std::vector<Node_Id_Type> const &successors) const
      -> H_g_collection;


    /// The basic function for the Eppstein algorithm
    /// \param K The number of shortest paths
    /// \param dij_res The result of dijkstra
    /// \param sidetrack_distances The collection of the sidetrack distances for all the sidetrack edges
    /// \return The k shortest paths
    [[nodiscard]] auto
    start(std::size_t                              K,
          Parent_Type::Dijkstra_Result_Type const &dij_res,
          Internal_Weight_Collection_Type const   &sidetrack_distances) const -> Output_Type override;

  public:
    explicit KFinder_Eppstein(GraphType const &g, std::size_t root, std::size_t sink)
      : Parent_Type(g, root, sink){};

    explicit KFinder_Eppstein(Weighted_Graph<GraphType> const &g, std::size_t root, std::size_t sink)
      : Parent_Type(g, root, sink){};

    ~KFinder_Eppstein() override = default;
  };


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::start(
    std::size_t                            K,
    Dijkstra_Result_Type const            &dij_res,
    Internal_Weight_Collection_Type const &sidetrack_distances) const -> Output_Type
  {
    auto const &successors = dij_res.first;

    auto h_out = construct_h_out(successors, sidetrack_distances); // O(E)
    auto h_g   = construct_h_g(h_out, successors);     // O(N*log(N))

    // O(K*log(K)) + O(Path_reconstruction)
    return Parent_Type::general_algo_eppstein(K, dij_res, sidetrack_distances, h_g, h_out);
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_h_out(
    std::vector<Node_Id_Type> const       &successors,
    Internal_Weight_Collection_Type const &sidetrack_distances) const -> H_out_collection
  {
    auto const &graph = Parent_Type::graph;

    H_out_collection h_out_collection;
    h_out_collection.reserve(graph.size());

    for (auto const &tail_node : graph)
      {
        auto const &tail           = tail_node.get_id();
        auto const &tail_successor = successors[tail];

        if (tail_successor == std::numeric_limits<Node_Id_Type>::max())
          continue;

        // The output neighbors of the current node
        auto const            &head_nodes = graph.get_output_nodes(tail);
        std::vector<Edge_Info> sidetrack_edges;
        sidetrack_edges.reserve(sidetrack_distances[tail].size());

        // Loop through the output neighbors of the current node
        for (auto const &head : head_nodes)
          {
            auto [begin, end] = sidetrack_distances[tail].equal_range(head);

            for (; begin != end && begin->first == head; ++begin)
              {
                sidetrack_edges.emplace_back(std::make_pair(tail, head), begin->second);
              }
          }

        // Add to the collection of H_outs
        h_out_collection.emplace_hint(h_out_collection.end(),
                                      tail,
                                      typename H_out_collection::mapped_type(std::move(sidetrack_edges)));
      }

    return h_out_collection;
  }


  template <class Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  KFinder_Eppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::construct_h_g(
    const H_out_collection          &h_out_collection,
    const std::vector<Node_Id_Type> &successors) const -> H_g_collection
  {
    auto const &num_nodes = graph.size();

    H_g_collection h_g_collection;
    h_g_collection.reserve(graph.size());

    // sp_dependencies contains the predecessors of every node in the shortest path.
    std::vector<std::set<Node_Id_Type>> sp_dependencies;
    sp_dependencies.resize(num_nodes);

    // At worst, since each node is contained in up to a single sp_dependencies, the loop takes at most O(N*log(N))
    for (auto const &node : graph) // O(N)
      {
        auto const &node_id   = node.get_id();
        auto const &successor = successors[node_id];

        if (successor == std::numeric_limits<Node_Id_Type>::max())
          continue;

        if (sink != node_id)
          sp_dependencies[successor].insert(node_id); // O(log(N))
      }

    // The actual generation of the H_g should now start from the sink node
    auto h_out_iterator = h_out_collection.find(sink); // O(1), it always exists

    // Prepare the last H_g
    h_g_collection.emplace(sink, typename H_g_collection::mapped_type(&h_out_iterator->second)); // O(1)

    // Now, we have to find the nodes whose successor is the sink itself
    std::queue<Node_Id_Type> queue;
    for (auto const &node : graph) // O(N)
      {
        auto const &id = node.get_id();
        if (successors[id] == sink && id != sink) // O(1)
          queue.push(id);
      }

    // Loop through the queue
    while (!queue.empty()) // O(N)
      {
        auto const &front_element = queue.front();

        // Find the "new" nodes that must be added to the queue.
        auto       &deps          = sp_dependencies[front_element];       // O(1)
        auto const &successor_h_g = h_g_collection.find(successors[front_element])->second; // O(1)

        h_g_collection.emplace(front_element,
                               typename H_g_collection::mapped_type(&(h_out_collection.find(front_element)->second),
                                                                    successor_h_g)); // O(log(N))


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
