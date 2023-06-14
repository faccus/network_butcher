#ifndef NETWORK_BUTCHER_BASIC_KEPPSTEIN_H
#define NETWORK_BUTCHER_BASIC_KEPPSTEIN_H

#include <functional>
#include <list>
#include <optional>
#include <ranges>
#include <variant>

#include <network_butcher/K-shortest_path/heap_traits.h>
#include <network_butcher/K-shortest_path/shortest_path_finder.h>

#include <network_butcher/APSC/chrono.h>
#include <network_butcher/K-shortest_path/kfinder.h>

namespace network_butcher::kfinder
{
  /// A (pure virtual) class that provides the common methods that are used by the different Eppstein algorithms.
  /// Complexity computations are referred to WGraph.
  /// \tparam GraphType The type of the graph
  /// \tparam Only_Distance A boolean that indicates if the algorithm should return only the distance or the full paths
  /// \tparam t_Weighted_Graph_Complete_Type The type of the weighted graph
  template <typename GraphType,
            bool                 Only_Distance,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<GraphType>>
  class Basic_KEppstein : public KFinder<GraphType, Only_Distance, t_Weighted_Graph_Complete_Type>
  {
  private:
    /// The parent type. Used to access quickly to the parent methods
    using Parent_Type = KFinder<GraphType, Only_Distance, t_Weighted_Graph_Complete_Type>;

  public:
    /// The type of the output of the algorithm
    using Output_Type = Parent_Type::Output_Type;

  protected:
    /// Bring forward the graph
    using Parent_Type::graph;

    /// Bring forward the root node id
    using Parent_Type::root;

    /// Bring forward the sink node id
    using Parent_Type::sink;

    /// Weight Type
    using Weight_Type = typename t_Weighted_Graph_Complete_Type::Weight_Type;

    /// Type for an edge with its weight
    using Edge_Info = Templated_Edge_Info<Weight_Type>;

    /// Type for a path, with its length
    using Path_Info = Templated_Path_Info<Weight_Type>;

    /// Type of the collection of H_g
    using H_g_collection = Templated_H_g_Collection<Weight_Type>;

    /// Type of the collection of H_out
    using H_out_collection = Templated_H_out_Collection<Weight_Type>;

    /// Type of collection of weights. Used to map edges to their sidetrack weights
    using Internal_Weight_Collection_Type = std::vector<std::multimap<Node_Id_Type, Weight_Type>>;

    /// Type of the result of the Dijkstra algorithm
    using Dijkstra_Result_Type =
      network_butcher::kfinder::Shortest_path_finder::Templated_Dijkstra_Result_Type<Weight_Type>;

    /// Type of the callback function used by general_algo_eppstein
    using Callback_Function =
      std::function<typename H_g_collection::const_iterator(H_g_collection &,
                                                            H_out_collection &,
                                                            Internal_Weight_Collection_Type const &,
                                                            typename Dijkstra_Result_Type::first_type const &,
                                                            Node_Id_Type,
                                                            t_Weighted_Graph_Complete_Type const &)>;

    /// A struct that contains the information about a node in the D(G) graph
    class D_G_Node
    {
    private:
      /// Node of H_g
      H_g_collection::mapped_type ::Node_Type const *h_g_node;

      /// Node of H_out
      H_out_collection ::mapped_type ::Node_Type const *h_out_node;

    public:
      /// Constructs a D_G_Node from an H_g. No test is performed on h_g_it (D_G_Node assumes that it contains a valid
      /// heap). Perform the appropriate checks before calling the constructor
      /// \param h_g_it The iterator to the h_g_it collection
      explicit D_G_Node(H_g_collection::const_iterator const &h_g_it)
        : h_g_node{h_g_it->second.get_head_node()}
        , h_out_node{nullptr} {};

      /// Constructs a D_G_Node from a node of H_g
      /// \param node The node of H_g
      explicit D_G_Node(H_g_collection::mapped_type ::Node_Type const *node)
        : h_g_node{node}
        , h_out_node{nullptr} {};

      /// Constructs a D_G_Node from a node of H_out
      /// \param node The node of H_out
      explicit D_G_Node(H_out_collection::mapped_type ::Node_Type const *node)
        : h_g_node{nullptr}
        , h_out_node{node} {};


      /// It will produce the children of the given node in the D(G) graph
      /// \return
      [[nodiscard]] auto
      get_children() const -> std::vector<D_G_Node>
      {
        std::vector<D_G_Node> res;
        if (!valid())
          return res;

        if (h_out_node)
          {
            auto const &node = h_out_node;

            // Add all the H_out related children
            for (auto const &child : node->get_children())
              res.emplace_back(child);
          }
        else
          {
            auto const &node = h_g_node;

            // It should be a single child of the head of H_out
            auto const &children = node->get_content()->get_head_node()->get_children();

            if (!children.empty())
              res.emplace_back(children.front());

            // Add all the H_g related children
            for (auto const &child : node->get_children())
              res.emplace_back(child);
          }

        return res;
      }

      /// Checks if either the node of H_g or H_out is not nullptr
      /// \return True if either of the two nodes is not nullptr
      [[nodiscard]] auto
      valid() const -> bool
      {
        return h_g_node || h_out_node;
      }

      /// Returns the sidetrack edge associated with the current D_G_Node node
      /// \return The sidetrack edge
      auto
      get_head_content() const -> Edge_Info const &
      {
        if (h_g_node)
          {
            return h_g_node->get_content()->get_head_content();
          }
        else if (h_out_node)
          {
            return h_out_node->get_content();
          }
        else
          {
            throw std::runtime_error("D_G_Node: No valid pointer was provided");
          }
      }
    };

    /// Simple struct to represent an implicit path
    struct Implicit_Path_Info : Crtp_Greater<Implicit_Path_Info>
    {
      /// The current sidetrack
      std::optional<D_G_Node> current_sidetrack;

      /// Pointer to a previous path. The overall path is made by the concatenation of the previous path with
      /// current_sidetrack
      Implicit_Path_Info *previous_sidetracks;

      /// The length of the path
      Weight_Type length;


      auto
      operator<(const Implicit_Path_Info &rhs) const -> bool
      {
        return length < rhs.length;
      }

      /// Recursive method to compute the sequence of sidetracks associated to the current path
      /// \param sidetracks The list of sidetracks
      void
      compute_sidetracks(std::list<Edge_Info const *> &sidetracks) const
      {
        if (!current_sidetrack)
          {
            return;
          }

        if (previous_sidetracks != nullptr)
          {
            previous_sidetracks->compute_sidetracks(sidetracks);
          }

        sidetracks.push_back(&(current_sidetrack.value().get_head_content()));
      }

      /// Produce the list of sidetrack edges associated to the current path
      /// \return The list of sidetracks
      [[nodiscard]] auto
      compute_sidetracks() const -> std::list<Edge_Info const *>
      {
        if (!current_sidetrack)
          {
            return {};
          }

        std::list<Edge_Info const *> sidetracks;
        if (previous_sidetracks != nullptr)
          {
            previous_sidetracks->compute_sidetracks(sidetracks);
          }

        sidetracks.push_back(&(current_sidetrack.value().get_head_content()));

        return sidetracks;
      }
    };

    /// Computes the sidetrack distances for all the different sidetrack edges
    /// \param dij_res The result of the Dijkstra algorithm
    /// \return The collection of sidetrack distances for the different edges
    [[nodiscard]] auto
    sidetrack_distances(Dijkstra_Result_Type const &dij_res) const -> Internal_Weight_Collection_Type;


    /// Helper function for the Eppstein algorithm. It converts a vector of implicit paths to a vector of explicit
    /// paths
    /// \param dij_res The result of the Dijkstra result
    /// \param epp_res The collection of implicit paths
    /// \return The shortest paths
    [[nodiscard]] auto
    helper_eppstein(Dijkstra_Result_Type const &dij_res, std::vector<Implicit_Path_Info> const &epp_res) const
      -> std::vector<Path_Info>;


    /// This function must be specialized by the different algorithms. It should prepare the required H_outs, H_gs
    /// and, if needed, prepare the callback function and, finally, call the general_algo_eppstein function
    /// \param K The number of shortest paths to compute
    /// \param dij_res The result of the Dijkstra algorithm
    /// \param sidetrack_distances The sidetrack distances of every sidetrack edge
    /// \return The shortest paths (in explicit form)
    [[nodiscard]] virtual auto
    start(std::size_t                            K,
          Dijkstra_Result_Type const            &dij_res,
          Internal_Weight_Collection_Type const &sidetrack_distances) const -> Output_Type = 0;


    /// The "general" structure of the Eppstein algorithms. It will construct the shortest paths
    /// \param K The number of shortest paths
    /// \param dij_res The result of the dijkstra algorithm
    /// \param sidetrack_distances The sidetrack distances of every edge
    /// \param h_g The h_g map
    /// \param h_out The h_out map
    /// \param callback_fun A callback function called during the loop used to find the shortest paths
    /// \return The (explicit) shortest paths
    auto
    general_algo_eppstein(std::size_t                            K,
                          Dijkstra_Result_Type const            &dij_res,
                          Internal_Weight_Collection_Type const &sidetrack_distances,
                          H_g_collection                        &h_g,
                          H_out_collection                      &h_out,
                          Callback_Function const               &callback_fun = nullptr) const -> Output_Type;

  public:
    /// Applies a K-shortest path algorithm to find the k-shortest paths on the given graph (from the root to the sink)
    /// \param K The number of shortest paths to find
    /// \return The shortest paths
    [[nodiscard]] auto
    compute(std::size_t K) const -> Output_Type override;

    /// Base class for Eppstein and Lazy Eppstein
    /// \param g The graph
    /// \param root The root node id
    /// \param sink The sink node id
    explicit Basic_KEppstein(t_Weighted_Graph_Complete_Type const &g, Node_Id_Type root, Node_Id_Type sink)
      : Parent_Type(g, root, sink){};


    /// Base class for Eppstein and Lazy Eppstein
    /// \param g The graph
    /// \param root The root node id
    /// \param sink The sink node id
    explicit Basic_KEppstein(GraphType const &g, Node_Id_Type root, Node_Id_Type sink)
      : Parent_Type(g, root, sink){};

    virtual ~Basic_KEppstein() = default;
  };


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::compute(std::size_t K) const
    -> Output_Type
  {
    if (graph.empty() || K == 0)
      return {};

    auto const dij_res = Shortest_path_finder::dijkstra(graph.reverse(), sink);

    // If the shortest path doesn't exist, then we can return an empty vector
    if (dij_res.second[root] == std::numeric_limits<Weight_Type>::max())
      return {};

    if (K == 1)
      {
        if constexpr (Only_Distance)
          {
            return {dij_res.second[root]};
          }
        else
          {
            return {Shortest_path_finder::shortest_path_finder(graph, dij_res, root, sink)};
          }
      }

    return start(K, dij_res, sidetrack_distances(dij_res));
  }


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::general_algo_eppstein(
    std::size_t                               K,
    Dijkstra_Result_Type const               &dij_res,
    Internal_Weight_Collection_Type const    &sidetrack_distances,
    H_g_collection                           &h_g,
    H_out_collection                         &h_out,
    const Basic_KEppstein::Callback_Function &callback_fun) const -> Output_Type
  {
    auto const &[successors, shortest_distance] = dij_res;

    // Start with the shortest path
    std::vector<Implicit_Path_Info> res;
    res.push_back(Implicit_Path_Info{.current_sidetrack   = std::optional<D_G_Node>(),
                                     .previous_sidetracks = nullptr,
                                     .length              = shortest_distance[root]});

    // Find the first sidetrack edge
    typename H_g_collection::const_iterator h_g_it = h_g.find(root);

    if (h_g_it->second.empty())
      {
        if constexpr (Only_Distance)
          {
            return {shortest_distance[root]};
          }
        else
          {
            return {Shortest_path_finder::shortest_path_finder(Parent_Type::graph, dij_res, root, Parent_Type::sink)};
          }
      }

    res.reserve(K);

    // Collection of "final" implicit paths to be added to res
    std::vector<Implicit_Path_Info> tmp_vect_queue;
    tmp_vect_queue.reserve(K);
    std::priority_queue<Implicit_Path_Info, std::vector<Implicit_Path_Info>, std::greater<>> Q(
      std::greater<>(), std::move(tmp_vect_queue));


    // First deviatory path
    D_G_Node first_side_track(h_g_it);
    Q.push(Implicit_Path_Info{.current_sidetrack   = first_side_track,
                              .previous_sidetracks = nullptr,
                              .length = first_side_track.get_head_content().delta_weight + shortest_distance[root]});


    std::size_t k = 2;

    // Loop through Q until either Q is empty, or the number of paths found is K
    for (; k <= K && !Q.empty(); ++k)
      {
        res.emplace_back(Q.top());
        Q.pop();

        auto const &SK                 = res.back();
        auto const &current_sidetrack  = SK.current_sidetrack.value();
        auto const &[e_edge, e_weight] = current_sidetrack.get_head_content();

        // "Helper" function that can be called if needed
        if (callback_fun != nullptr)
          {
            h_g_it = callback_fun(h_g, h_out, sidetrack_distances, successors, e_edge.second, graph);
          }
        else
          {
            h_g_it = h_g.find(e_edge.second);
          }

        if (!h_g_it->second.empty())
          {
            // Extract the first sidetrack edge if it exists
            D_G_Node f(h_g_it);
            Q.push(Implicit_Path_Info{.current_sidetrack   = f,
                                      .previous_sidetracks = &res.back(),
                                      .length = SK.length + f.get_head_content().delta_weight}); // O(log(K)
          }

        // O(1), there are up tp 3 elements in this collection
        for (auto const &sidetrack_edge : current_sidetrack.get_children())
          {
            Q.push(Implicit_Path_Info{.current_sidetrack   = sidetrack_edge,
                                      .previous_sidetracks = SK.previous_sidetracks,
                                      .length = SK.length + sidetrack_edge.get_head_content().delta_weight -
                                                e_weight}); // O(log(K))
          }
      }

    if constexpr (Only_Distance)
      {
        std::vector<Weight_Type> final_res;
        final_res.reserve(res.size());

        for (auto const &path : res)
          final_res.push_back(path.length);

        return final_res;
      }
    else
      {
        return helper_eppstein(dij_res, res);
      }
  }


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::helper_eppstein(
    Dijkstra_Result_Type const            &dij_res,
    std::vector<Implicit_Path_Info> const &epp_res) const -> std::vector<Path_Info>
  {
    auto const &root = Parent_Type::root;
    auto const &sink = Parent_Type::sink;

    auto const &successors = dij_res.first;

    std::vector<Path_Info> res(epp_res.size());

    auto const go_shortest = [&successors, sink](Node_Id_Type node) {
      std::vector<Node_Id_Type> final_steps;
      while (node != sink)
        {
          final_steps.push_back(node);
          node = successors[node];
        }

      final_steps.push_back(sink);
      return final_steps;
    };


    // Basically, we start from the specified node and go along the shortest path until we meet a sidetrack edge
    // contained in the implicit path. In that case, we add the sidetrack edge and proceed along the "new" shortest
    // path until either the "sink" node is reached or another sidetrack edge is met
    auto const process_path =
      [&go_shortest, &dij_res = dij_res, &res = res, &epp_res = epp_res, &root, &sink](std::size_t i) {
        auto const &implicit_path = epp_res[i];

        auto       &info       = res[i];
        auto const &sidetracks = implicit_path.compute_sidetracks();

        info.length = implicit_path.length;

        if (sidetracks.empty())
          {
            info.path = go_shortest(root);
          }
        else
          {
            auto sidetrack_edge_it = sidetracks.cbegin();

            auto [first, second]       = (*sidetrack_edge_it)->edge;
            std::size_t node_to_insert = root;

            while (node_to_insert != sink)
              {
                info.path.push_back(node_to_insert);
                if (first == node_to_insert)
                  {
                    node_to_insert = second;
                    ++sidetrack_edge_it;

                    if (sidetrack_edge_it == sidetracks.cend())
                      {
                        auto to_insert = go_shortest(node_to_insert);
                        info.path.insert(info.path.end(),
                                         std::make_move_iterator(to_insert.begin()),
                                         std::make_move_iterator(to_insert.end()));

                        break;
                      }

                    auto tmp = (*sidetrack_edge_it)->edge;
                    first    = tmp.first;
                    second   = tmp.second;
                  }
                else
                  node_to_insert = dij_res.first[node_to_insert];
              }
          }
      };
#if NETWORK_BUTCHER_PARALLEL_TBB
    // https://stackoverflow.com/a/63340360 .
    // Tested views, but they are slower, so they are not used.

    std::vector<std::size_t> v(epp_res.size());
    std::generate(v.begin(), v.end(), [n = 0]() mutable { return n++; });

    std::for_each(std::execution::par, v.begin(), v.end(), process_path);
#else
#  pragma omp parallel default(none) shared(go_shortest, dij_res, res, epp_res, root, sink, process_path)
    {
#  pragma omp for
      for (std::size_t i = 0; i < epp_res.size(); ++i)
        {
          process_path(i);
        }
    }
#endif

    return res;
  }


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::sidetrack_distances(
    Dijkstra_Result_Type const &dij_res) const -> Internal_Weight_Collection_Type
  {
    Internal_Weight_Collection_Type res;
    res.resize(graph.size());

    auto const &[successors, distances_from_sink] = dij_res;

    for (auto const &tail_node : graph) // The overall loop has worst case complexity O(E)
      {
        auto const &tail = tail_node.get_id();
        if (distances_from_sink[tail] == std::numeric_limits<Weight_Type>::max())
          continue;

        for (auto const &head : graph.get_output_nodes(tail))
          {
            if (distances_from_sink[head] == std::numeric_limits<Weight_Type>::max())
              continue;

            auto const  edge    = std::make_pair(tail, head);
            auto const &weights = graph.get_weight(edge);

            // If it is its successor...
            if (successors[tail] == head)
              {
                // ...and it has more than one weight, then we have to consider the edge with the smallest weight as the
                // edge in the shortest path tree, while the other ones will be sidetrack edges.
                if (weights.size() > 1)
                  {
                    bool found = false;

                    for (auto it = ++weights.cbegin(); it != weights.cend(); ++it)
                      {
                        auto const &weight = *it;

                        res[tail].emplace_hint(res[tail].cend(),
                                               head,
                                               weight + distances_from_sink[head] - distances_from_sink[tail]); // O(1)}
                      }
                  }
              }
            else
              {
                for (auto const &weight : weights)
                  {
                    res[tail].emplace_hint(res[tail].cend(),
                                           head,
                                           weight + distances_from_sink[head] - distances_from_sink[tail]); // O(1)
                  }
              }
          }
      }

    return res;
  }
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_BASIC_KEPPSTEIN_H