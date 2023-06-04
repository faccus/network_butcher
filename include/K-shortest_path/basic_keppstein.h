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

  protected:
    using Parent_Type::graph;

    using Weight_Type = typename t_Weighted_Graph_Complete_Type::Weight_Type;


    using Edge_Info = Templated_Edge_Info<Weight_Type>;
    using Path_Info = Templated_Path_Info<Weight_Type>;

    using H_g_collection   = Templated_H_g_Collection<Weight_Type>;
    using H_out_collection = Templated_H_out_Collection<Weight_Type>;


    /// (Position of H_out in H_g, Position of sidetrack in H_out)
    using Location_DG_Type                = std::pair<std::size_t, std::size_t>;
    using Internal_Weight_Collection_Type = std::multimap<Edge_Type, Weight_Type>;
    using Dijkstra_Result_Type =
      network_butcher::kfinder::Shortest_path_finder::Templated_Dijkstra_Result_Type<Weight_Type>;

    /// Sidetrack edge can be represented as two elements: the relevant H_g and the position of the edge in H_g as two
    /// integers, one for the position in H_g and one for the position in H_out
    struct Sidetrack
    {
      H_g_collection::const_iterator current_h_g;
      Location_DG_Type               location;
      Weight_Type                    delta_weight;

      bool
      operator<(Sidetrack const &rhs) const
      {
        return current_h_g->first < rhs.current_h_g->first ||
               (current_h_g->first == rhs.current_h_g->first && location < rhs.location);
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
      compute_sidetracks(std::list<Sidetrack const *> &sidetracks) const
      {
        if (!current_sidetrack)
          {
            return;
          }

        if (previous_sidetracks != nullptr)
          {
            previous_sidetracks->compute_sidetracks(sidetracks);
          }

        sidetracks.push_back(&current_sidetrack.value());
      }

      std::list<Sidetrack const *>
      compute_sidetracks() const
      {
        if (!current_sidetrack)
          {
            return {};
          }

        std::list<Sidetrack const *> sidetracks;
        if (previous_sidetracks != nullptr)
          {
            previous_sidetracks->compute_sidetracks(sidetracks);
          }

        sidetracks.push_back(&current_sidetrack.value());

        return sidetracks;
      }
    };

  public:
    using Output_Type = Parent_Type::Output_Type;

  protected:
    using Callback_Function =
      std::function<typename H_g_collection::const_iterator(H_g_collection &,
                                                            H_out_collection &,
                                                            Internal_Weight_Collection_Type const &,
                                                            typename Dijkstra_Result_Type::first_type const &,
                                                            Node_Id_Type)>;


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


    /// It contains the children of the given edge in the D(G) graph
    /// \param h_g_it H_g iterator
    /// \param position The location of the sidetrack edge in the H_g
    /// \return The children of the given edge in D(G)
    [[nodiscard]] auto
    get_alternatives(typename H_g_collection::const_iterator const &h_g_it, Location_DG_Type const &position) const
      -> std::vector<Sidetrack>;


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
    start(std::size_t K, Dijkstra_Result_Type const &dij_res) const -> Output_Type = 0;


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

    auto const dij_res = Shortest_path_finder::dijkstra(this->graph.reverse(), sink); // time:

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

    return start(K, dij_res);
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

    if (h_g_it == h_g.end() || h_g_it->second.empty())
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

    auto const &first_side_track = extract_first_sidetrack_edge(h_g_it);

    res.reserve(K);

    // Collection of "final" implicit paths to be added to res
    std::multiset<Implicit_Path_Info> Q;

    // First deviatory path
    Q.insert(Implicit_Path_Info{.current_sidetrack   = first_side_track,
                              .previous_sidetracks = nullptr,
                              .length              = first_side_track.delta_weight + shortest_distance[root]});

    std::size_t k = 2;
    // Loop through Q until either Q is empty or the number of paths found is K
    for (; k <= K && !Q.empty(); ++k) // O(K)
      {
        res.emplace_back(*Q.cbegin());
        Q.erase(Q.begin());

        auto const &SK = res.back();

        auto const &[current_h_g, current_location, _e_weight] = *SK.current_sidetrack;

        auto const &e_h_out            = current_h_g->second.get_elem(current_location.first);
        auto const &[e_edge, e_weight] = current_location.second == std::numeric_limits<Node_Id_Type>::max() ?
                                           e_h_out->second.get_elem(0) :
                                           e_h_out->second.get_elem(current_location.second);

        // "Helper" function that can be called if needed
        if (callback_fun != nullptr)
          {
            h_g_it = callback_fun(h_g, h_out, sidetrack_distances, successors, e_edge.second);
          }
        else
          {
            h_g_it = h_g.find(e_edge.second);
          }

        if (h_g_it != h_g.end() && !h_g_it->second.empty())
          {
            // Extract the first sidetrack edge, if it exists
            auto const &f = extract_first_sidetrack_edge(h_g_it);

            Q.insert(Implicit_Path_Info{.current_sidetrack   = f,
                                        .previous_sidetracks = &res.back(),
                                        .length              = SK.length + f.delta_weight}); // O(log(K))
          }

        auto const alternatives = get_alternatives(current_h_g, current_location);

        if (!alternatives.empty())
          {
            for (auto const &sidetrack_edge : alternatives)
              {
                // O(log(K))
                Q.insert(Implicit_Path_Info{.current_sidetrack   = sidetrack_edge,
                                            .previous_sidetracks = SK.previous_sidetracks,
                                            .length              = SK.length + sidetrack_edge.delta_weight - e_weight});
              }
          }

        // O(1), since every time up to 4 paths may be inserted. Thus, the loop can run up to 3 times
        while (Q.size() > K)
          {
            Q.erase((++Q.rbegin()).base());
          }
      }

    if constexpr (Only_Distance)
      {
        std::vector<Weight_Type> final_res;
        final_res.reserve(res.size());

        for (auto const &path : res) // O(K)
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

    auto const extract_edge = [](H_out_collection::const_iterator h_out, Location_DG_Type const &loc) {
      return h_out->second.get_elem(loc.second).edge;
    };

    // Basically, we start from the specified node and go along the shortest path until we meet a sidetrack edge
    // contained in the implicit path. In that case, we add the sidetrack edge and proceed along the "new" shortest
    // path until either the "sink" node is reached or another sidetrack edge is met
#if PARALLEL_TBB
    auto const process_path =
      [&go_shortest, &extract_edge, &dij_res = dij_res, &res = res, &epp_res = epp_res, &root, &sink](
        std::size_t index) {
        auto const &implicit_path = epp_res[index];

        auto       &info       = res[index];
        auto const &sidetracks = implicit_path.compute_sidetracks();

        info.length = implicit_path.length;

        if (sidetracks.empty())
          {
            info.path = go_shortest(root);
          }
        else
          {
            auto        it             = sidetracks.cbegin();
            std::size_t node_to_insert = root;


            auto h_out_pos       = (*it)->current_h_g->second.get_elem((*it)->location.first);
            auto [first, second] = extract_edge(h_out_pos, (*it)->location);

            while (node_to_insert != sink)
              {
                info.path.push_back(node_to_insert);
                if (first == node_to_insert)
                  {
                    node_to_insert = second;
                    ++it;

                    if (it == sidetracks.cend())
                      {
                        auto to_insert = go_shortest(node_to_insert);
                        info.path.insert(info.path.end(),
                                         std::make_move_iterator(to_insert.begin()),
                                         std::make_move_iterator(to_insert.end()));

                        break;
                      }

                    h_out_pos = (*it)->current_h_g->second.get_elem((*it)->location.first);

                    auto tmp = extract_edge(h_out_pos, (*it)->location);
                    first    = tmp.first;
                    second   = tmp.second;
                  }
                else
                  node_to_insert = dij_res.first[node_to_insert];
              }
          }
      };

    auto const &view = std::ranges::iota_view(std::size_t{0}, epp_res.size());
    std::for_each(std::execution::par, view.begin(), view.end(), process_path);
#else

#  pragma omp parallel default(none) shared(epp_res, res, go_shortest, extract_edge, root, sink, dij_res)
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
              auto        it             = sidetracks.cbegin();
              std::size_t node_to_insert = root;


              auto h_out_pos       = (*it)->current_h_g->second.get_elem((*it)->location.first);
              auto [first, second] = extract_edge(h_out_pos, (*it)->location);

              while (node_to_insert != sink)
                {
                  info.path.push_back(node_to_insert);
                  if (first == node_to_insert)
                    {
                      node_to_insert = second;
                      ++it;

                      if (it == sidetracks.cend())
                        {
                          auto to_insert = go_shortest(node_to_insert);
                          info.path.insert(info.path.end(),
                                           std::make_move_iterator(to_insert.begin()),
                                           std::make_move_iterator(to_insert.end()));

                          break;
                        }

                      h_out_pos = (*it)->current_h_g->second.get_elem((*it)->location.first);

                      auto tmp = extract_edge(h_out_pos, (*it)->location);
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
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::get_alternatives(
    H_g_collection::const_iterator const &h_g_it,
    Location_DG_Type const               &position) const -> std::vector<Sidetrack>
  {
    std::vector<Sidetrack> res;
    auto constexpr inf = std::numeric_limits<Node_Id_Type>::max();

    auto [h_out_index, index] = position;
    auto const &h_g           = h_g_it->second;

    // In this case, edge is a head element of an H_out in H_g. Thus, I have to find its children in H_g and its child
    // in H_out (since it's the head of an H_out, index can be set to 0).
    if (index == 0)
      {
        for (auto const &el : h_g.find_children_indices(h_out_index))
          {
            auto const  location_dg_type = std::make_pair(el, 0);
            auto const &h_g_child        = h_g.get_elem(el);

            if (!h_g_child->second.empty())
              res.emplace_back(h_g_it, location_dg_type, h_g_child->second.get_head().delta_weight);
          }

        index = 0;
      }

    auto const &h_out = h_g.get_elem(h_out_index);
    for (auto const &el : h_out->second.find_children_indices(index))
      {
        auto const location_dg_type = std::make_pair(h_out_index, el);
        res.emplace_back(h_g_it, location_dg_type, h_out->second.get_elem(el).delta_weight);
      }

    return res;
  }


  template <typename Graph_type, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  auto
  Basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::sidetrack_distances(
    Dijkstra_Result_Type const &dij_res) const -> Internal_Weight_Collection_Type
  {
    Internal_Weight_Collection_Type res;
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

                        res.insert(res.cend(),
                                   {edge, weight + distances_from_sink[head] - distances_from_sink[tail]}); // O(1)}
                      }
                  }
              }
            else
              {
                for (auto const &weight : weights)
                  {
                    res.insert(res.cend(),
                               {edge, weight + distances_from_sink[head] - distances_from_sink[tail]}); // O(1)}
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
    auto const &edge = h_g_it->second.get_elem(0)->second.get_elem(0);

    return Sidetrack{.current_h_g = h_g_it, .location = std::make_pair(0, 0), .delta_weight = edge.delta_weight};
  }
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_BASIC_KEPPSTEIN_H
