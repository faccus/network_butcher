//
// Created by faccus on 21/04/23.
//

#ifndef NETWORK_BUTCHER_BASIC_KEPPSTEIN_H
#define NETWORK_BUTCHER_BASIC_KEPPSTEIN_H

#include <functional>
#include <list>
#include <optional>
#include <ranges>

#include "heap_traits.h"
#include "shortest_path_finder.h"

#include "chrono.h"
#include "kfinder.h"

namespace network_butcher::kfinder
{
  /// A (pure virtual) class that provides the common methods that are used by the different Eppstein algorithms
  template <typename GraphType,
            bool                 Only_Distance,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<GraphType>>
  class Basic_KEppstein : public KFinder<GraphType, Only_Distance, t_Weighted_Graph_Complete_Type>
  {
  private:
    using Parent_Type = KFinder<GraphType, Only_Distance, t_Weighted_Graph_Complete_Type>;

  public:
    using Output_Type = Parent_Type::Output_Type;

  protected:
    using Parent_Type::graph;

    using Weight_Type = typename t_Weighted_Graph_Complete_Type::Weight_Type;


    using Edge_Info = Templated_Edge_Info<Weight_Type>;
    using Path_Info = Templated_Path_Info<Weight_Type>;

    using H_g_collection   = Templated_H_g_Collection<Weight_Type>;
    using H_out_collection = Templated_H_out_Collection<Weight_Type>;


    using Internal_Weight_Collection_Type = std::vector<std::multimap<Node_Id_Type, Weight_Type>>;
    using Dijkstra_Result_Type =
      network_butcher::kfinder::Shortest_path_finder::Templated_Dijkstra_Result_Type<Weight_Type>;

    using Callback_Function =
      std::function<typename H_g_collection::const_iterator(H_g_collection &,
                                                            H_out_collection &,
                                                            Internal_Weight_Collection_Type const &,
                                                            typename Dijkstra_Result_Type::first_type const &,
                                                            Node_Id_Type,
                                                            t_Weighted_Graph_Complete_Type const &)>;

    /// A struct that contains the information about a sidetrack
    class Sidetrack
    {
    private:
      H_g_collection::mapped_type ::Node_Type const    *h_g_node;
      H_out_collection ::mapped_type ::Node_Type const *h_out_node;

    public:
      explicit Sidetrack(H_g_collection::mapped_type ::Node_Type const *node)
        : h_g_node{node}
        , h_out_node{nullptr} {};

      explicit Sidetrack(H_out_collection::mapped_type ::Node_Type const *node)
        : h_g_node{nullptr}
        , h_out_node{node} {};

      [[nodiscard]] auto
      get_node() const -> std::variant<typename H_g_collection::mapped_type ::Node_Type const *,
                                       typename H_out_collection ::mapped_type ::Node_Type const *>
      {
        if (h_g_node)
          {
            return h_g_node;
          }
        else if (h_out_node)
          {
            return h_out_node;
          }
        else
          {
            throw std::runtime_error("Sidetrack: No valid pointer was provided");
          }
      }

      [[nodiscard]] auto
      valid() const -> bool
      {
        return h_g_node || h_out_node;
      }

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
            throw std::runtime_error("Sidetrack: No valid pointer was provided");
          }
      }
    };

    /// Simple struct to represent an implicit path
    struct Implicit_Path_Info : Crtp_Greater<Implicit_Path_Info>
    {
      std::optional<Sidetrack> current_sidetrack;
      Implicit_Path_Info      *previous_sidetracks; // Previous Implicit_Path_Info in the chain
      Weight_Type              length;

      bool
      operator<(const Implicit_Path_Info &rhs) const
      {
        return length < rhs.length;
      }

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

      std::list<Edge_Info const *>
      compute_sidetracks() const
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


    /// It extracts the first sidetrack associated to the given node
    /// \param h_g_it The h_g map iterator
    /// \return The corresponding sidetrack edge
    [[nodiscard]] auto
    extract_first_sidetrack_edge(typename H_g_collection::const_iterator const &h_g_it) const -> Sidetrack;

    /// Computes the sidetrack distances for all the different sidetrack edges
    /// \param dij_res The result of the Dijkstra algorithm
    /// \return The collection of sidetrack distances for the different edges
    [[nodiscard]] auto
    sidetrack_distances(Dijkstra_Result_Type const &dij_res) const -> Internal_Weight_Collection_Type;


    /// Given the input sidetrack, it will return the vector of next alternative sidetracks
    /// \param current_sidetrack The current sidetrack
    /// \return The vector of next alternative sidetracks
    [[nodiscard]] auto
    get_alternatives(Sidetrack const &current_sidetrack) const -> std::vector<Sidetrack>;


    /// Helper function for the Eppstein algorithm. It converts a vector of implicit paths to a vector of explicit
    /// paths
    /// \param dij_res The result of the Dijkstra result
    /// \param epp_res The collection of implicit paths
    /// \return The shortest paths
    [[nodiscard]] auto
    helper_eppstein(Dijkstra_Result_Type const &dij_res, std::vector<Implicit_Path_Info> const &epp_res) const
      -> std::vector<Path_Info>;


    /// This function must be specialized by the different algorithms. It should prepare the required H_outs and H_gs
    /// and call the general_algo_eppstein function
    /// \param K The number of shortest paths to compute
    /// \param dij_res The result of the Dijkstra algorithm
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
    /// \return The (implicit) shortest paths
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

    explicit Basic_KEppstein(Weighted_Graph<GraphType> const &g, std::size_t root, std::size_t sink)
      : Parent_Type(g, root, sink){};

    explicit Basic_KEppstein(GraphType const &g, std::size_t root, std::size_t sink)
      : Parent_Type(g, root, sink){};

    virtual ~Basic_KEppstein() = default;
  };


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::compute(std::size_t K) const
    -> Output_Type
  {
    auto const &graph = Parent_Type::graph;
    auto const &root  = Parent_Type::root;
    auto const &sink  = Parent_Type::sink;

    if (graph.empty() || K == 0)
      return {};

#if PRINT_DEBUG_STATEMENTS
    Chrono dd_crono;
    dd_crono.start();
#endif

    auto const dij_res = Shortest_path_finder::dijkstra(this->graph.reverse(), sink); // time:

#if PRINT_DEBUG_STATEMENTS
    dd_crono.stop();
    std::cout << "basic_KEppstein, dijkstra: " << dd_crono.wallTime() / 1000. << " ms" << std::endl;
#endif

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

#if PRINT_DEBUG_STATEMENTS
    dd_crono.start();
#endif

    auto const sidetrack_distances_res = sidetrack_distances(dij_res); // O(E)

#if PRINT_DEBUG_STATEMENTS
    dd_crono.stop();
    std::cout << "basic_KEppstein, sidetrack_distances_computation: " << dd_crono.wallTime() / 1000. << " ms"
              << std::endl;
#endif

    return start(K, dij_res, sidetrack_distances_res);
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
    auto const &root   = Parent_Type::root;
    auto constexpr inf = std::numeric_limits<Node_Id_Type>::max();

    auto const &[successors, shortest_distance] = dij_res;

    // If the shortest path doesn't exist, then we can return an empty vector
    if (shortest_distance[root] == std::numeric_limits<Weight_Type>::max())
      return {};

    // Start with the shortest path
    std::vector<Implicit_Path_Info> res;
    res.push_back(Implicit_Path_Info{.current_sidetrack   = std::optional<Sidetrack>(),
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
    auto const &first_side_track = extract_first_sidetrack_edge(h_g_it);


    Q.push(Implicit_Path_Info{.current_sidetrack   = first_side_track,
                              .previous_sidetracks = nullptr,
                              .length = first_side_track.get_head_content().delta_weight + shortest_distance[root]});


    std::size_t k = 2;

    // Loop through Q until either Q is empty or the number of paths found is K
    for (; k <= K && !Q.empty(); ++k) // O(K)
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
            h_g_it = h_g.find(e_edge.second); // O(1), the element should always exist!
          }

        if (!h_g_it->second.empty())
          {
            // Extract the first sidetrack edge, if it exists
            auto const &f = extract_first_sidetrack_edge(h_g_it);
            Q.push(Implicit_Path_Info{.current_sidetrack   = f,
                                      .previous_sidetracks = &res.back(),
                                      .length = SK.length + f.get_head_content().delta_weight}); // O(log(K)
          }
        auto const &sidetrack_edges = get_alternatives(current_sidetrack);

        // O(1), there are up tp 3 elements in this collection
        for (auto const &sidetrack_edge : sidetrack_edges)
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

        for (auto &path : res) // O(K)
          final_res.emplace_back(std::move(path.length));

        return final_res;
      }
    else
      {
        return helper_eppstein(dij_res, res);
      }
  }


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::get_alternatives(
    Sidetrack const &current_sidetrack) const -> std::vector<Sidetrack>
  {
    using H_out_ptr = typename H_out_collection::mapped_type::Node_Type const *;
    using H_g_ptr   = typename H_g_collection::mapped_type::Node_Type const *;

    std::vector<Sidetrack> res;
    if (!current_sidetrack.valid())
      return res;

    auto const &variant_node = current_sidetrack.get_node();
    if (std::holds_alternative<H_out_ptr>(variant_node))
      {
        auto const &node = std::get<H_out_ptr>(variant_node);

        // Add all the H_out related children
        for (auto const &child : node->get_children())
          res.emplace_back(child);
      }
    else
      {
        auto const &node = std::get<H_g_ptr>(variant_node);


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


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::helper_eppstein(
    Dijkstra_Result_Type const            &dij_res,
    std::vector<Implicit_Path_Info> const &epp_res) const -> std::vector<Path_Info>
  {
    auto const &root = Parent_Type::root;
    auto const &sink = Parent_Type::sink;

    auto const &successors = dij_res.first;
    auto const &distances  = dij_res.second;

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
#if PARALLEL_TBB
    auto const process_path =
      [&go_shortest, &dij_res = dij_res, &res = res, &epp_res = epp_res, &root, &sink](std::size_t index) {
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
      }
  };

  auto const &view = std::ranges::iota_view(std::size_t{0}, epp_res.size());
  std::for_each(std::execution::par, view.begin(), view.end(), process_path);
#else

#  pragma omp parallel default(none) shared(epp_res, res, go_shortest, root, sink, dij_res)
    {
#  pragma omp for
      for (std::size_t i = 0; i < epp_res.size(); ++i)
        {
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

  for (auto const &tail_node : graph)
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
                      auto const &weight    = *it;
                      auto        to_insert = weight + distances_from_sink[head] - distances_from_sink[tail];

                      res[edge.first].emplace(edge.second, std::move(to_insert)); // O(1)}
                    }
                }
            }
          else
            {
              for (auto const &weight : weights)
                {
                  auto to_insert = weight + distances_from_sink[head] - distances_from_sink[tail];
                  res[edge.first].emplace(edge.second, std::move(to_insert)); // O(1)
                }
            }
        }
    }

  return res;
}


template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
auto
Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::extract_first_sidetrack_edge(
  H_g_collection::const_iterator const &h_g_it) const -> Sidetrack
{
  return Sidetrack(h_g_it->second.get_head_node());
}
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_BASIC_KEPPSTEIN_H
