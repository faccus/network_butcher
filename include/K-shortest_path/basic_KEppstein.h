//
// Created by faccus on 21/04/23.
//

#ifndef NETWORK_BUTCHER_BASIC_KEPPSTEIN_H
#define NETWORK_BUTCHER_BASIC_KEPPSTEIN_H

#include <functional>

#include "Heap_traits.h"
#include "Shortest_path_finder.h"

#include "KFinder.h"

namespace network_butcher::kfinder
{
  /// A (pure virtual) class that provides the common methods that are used by the different Eppstein algorithms
  template <typename Graph_type,
            bool                 Only_Distance,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<Graph_type>>
  class basic_KEppstein : public KFinder<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>
  {
  protected:
    using base = KFinder<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>;

    using Weight_Type = typename t_Weighted_Graph_Complete_Type::Weight_Type;

    using base::graph;

    using edge_info = t_edge_info<Weight_Type>;
    using path_info = t_path_info<Weight_Type>;

    using H_g_collection   = t_H_g_collection<Weight_Type>;
    using H_out_collection = t_H_out_collection<Weight_Type>;


    // (Position of H_out in H_g, Position of sidetrack in H_out)
    using location_dg_type = std::pair<std::size_t, std::size_t>;
    using element_dg_type  = std::pair<edge_info, location_dg_type>;
    using elements_dg_type = std::vector<element_dg_type>;

    using internal_weight_collection = std::multimap<edge_type, Weight_Type>;
    using dijkstra_result_type = network_butcher::kfinder::Shortest_path_finder::dijkstra_result_type<Weight_Type>;


    struct sidetrack
    {
      H_g_collection::const_iterator current_h_g;
      location_dg_type               location;
      Weight_Type                    delta_weight;

      template <typename A, typename B, typename C>
      sidetrack(A &&current_h_g, B &&location, C &&delta_weight)
        requires std::is_convertible_v<A, typename H_g_collection::const_iterator> &&
                   std::is_convertible_v<B, location_dg_type> && std::is_convertible_v<C, Weight_Type>
        : current_h_g(std::forward<A>(current_h_g))
        , location(std::forward<B>(location))
        , delta_weight(std::forward<C>(delta_weight))
      {}

      bool
      operator<(sidetrack const &rhs) const
      {
        return current_h_g->first < rhs.current_h_g->first ||
               (current_h_g->first == rhs.current_h_g->first && location < rhs.location);
      }
    };

    /// Simple struct to represent an implicit path
    struct implicit_path_info : crtp_greater<implicit_path_info>
    {
      std::vector<sidetrack> sidetracks;
      Weight_Type            length;

      template <typename A, typename B>
      implicit_path_info(A &&sidetracks, B &&length)
        requires std::is_convertible_v<A, std::vector<sidetrack>> && std::is_convertible_v<B, Weight_Type>
        : sidetracks(std::forward<A>(sidetracks))
        , length(std::forward<B>(length))
      {}

      bool
      operator<(const implicit_path_info &rhs) const
      {
        return length < rhs.length || (length == rhs.length && sidetracks < rhs.sidetracks);
      }
    };

  public:
    using Output_Type = std::conditional_t<Only_Distance, std::vector<Weight_Type>, std::vector<path_info>>;

  protected:
    using algo_output = std::conditional_t<Only_Distance, std::vector<Weight_Type>, std::vector<implicit_path_info>>;

    using callback_function =
      std::function<typename H_g_collection::const_iterator(H_g_collection &,
                                                            H_out_collection &,
                                                            internal_weight_collection const &,
                                                            typename dijkstra_result_type::first_type const &,
                                                            node_id_type)>;


    /// It extracts the first sidetrack associated to the given node
    /// \param h_g_it The h_g map iterator
    /// \return The corresponding sidetrack edge
    [[nodiscard]] sidetrack
    extract_first_sidetrack_edge(typename H_g_collection::const_iterator const &h_g_it) const
    {
      auto const &edge = h_g_it->second.get_elem(0)->second.get_elem(0);
      return {h_g_it, std::make_pair(0, std::numeric_limits<node_id_type>::max()), edge.delta_weight};
    }

    /// Computes the sidetrack distances for all the different sidetrack edges
    /// \param dij_res The result of the Dijkstra algorithm
    /// \return The collection of sidetrack distances for the different edges
    [[nodiscard]] internal_weight_collection
    sidetrack_distances(dijkstra_result_type const &dij_res) const
    {
      internal_weight_collection res;
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

              if (successors[tail] == head)
                {
                  if (weights.size() > 1)
                    {
                      bool found = false;
                      for (auto const &weight : weights)
                        {
                          auto to_insert = weight + distances_from_sink[head] - distances_from_sink[tail];

                          if (!found && to_insert == 0)
                            {
                              found = true;
                              continue;
                            }

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


    /// It contains the children of the given edge in the D(G) graph
    /// \param h_g_it H_g iterator
    /// \param position The location of the sidetrack edge in the H_g
    /// \return The children of the given edge in D(G)
    [[nodiscard]] std::vector<sidetrack>
    get_alternatives(typename H_g_collection::const_iterator const &h_g_it, location_dg_type const &position) const
    {
      std::vector<typename basic_KEppstein<Graph_type, Only_Distance, t_Weighted_Graph_Complete_Type>::sidetrack> res;
      auto constexpr inf = std::numeric_limits<node_id_type>::max();

      auto [h_out_index, index] = position;
      auto const &h_g           = h_g_it->second;

      // In this case, edge is a head element of an H_out in H_g. Thus, I have to find its children in H_g and its child
      // in H_out (since it's the head of an H_out, index can be set to 0).
      if (index == inf)
        {
          for (auto const &el : h_g.find_children_indices(h_out_index))
            {
              auto const  location_dg_type = std::make_pair(el, inf);
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


    /// Helper function for the Eppstein algorithm. It converts a vector of implicit paths to a vector of explicit
    /// paths
    /// \param dij_res The result of the Dijkstra result
    /// \param epp_res The collection of implicit paths
    /// \return The shortest paths
    [[nodiscard]] std::vector<path_info>
    helper_eppstein(dijkstra_result_type const &dij_res, std::vector<implicit_path_info> const &epp_res) const
    {
      auto const &root = base::root;
      auto const &sink = base::sink;

      auto const &[successors, distances] = dij_res;

      std::vector<path_info> res;
      res.reserve(epp_res.size());

      auto const go_shortest = [&successors, sink](node_id_type node) {
        std::vector<node_id_type> final_steps;
        while (node != sink)
          {
            final_steps.push_back(node);
            node = successors[node];
          }

        final_steps.push_back(sink);
        return final_steps;
      };

      auto const extrack_edge = [](H_out_collection::const_iterator h_out, location_dg_type const &loc) {
        if (loc.second != std::numeric_limits<node_id_type>::max())
          {
            return h_out->second.get_elem(loc.second).edge;
          }
        else
          {
            return h_out->second.get_head().edge;
          }
      };

      // Basically, we start from the specified node and go along the shortest path until we meet a sidetrack edge
      // contained in the implicit path. In that case, we add the sidetrack edge and proceed along the "new" shortest
      // path until either the "sink" node is reached or another sidetrack edge is met
      for (const auto &implicit_path : epp_res)
        {
          path_info info;
          info.length = implicit_path.length;

          auto const &sidetracks = implicit_path.sidetracks;
          if (sidetracks.empty())
            {
              info.path = go_shortest(root);
            }
          else
            {
              auto        it             = sidetracks.cbegin();
              std::size_t node_to_insert = root;

              auto h_out_pos       = it->current_h_g->second.get_elem(it->location.first);
              auto [first, second] = extrack_edge(h_out_pos, it->location);

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

                      h_out_pos = it->current_h_g->second.get_elem(it->location.first);

                      auto tmp = extrack_edge(h_out_pos, it->location);
                      first    = tmp.first;
                      second   = tmp.second;
                    }
                  else
                    node_to_insert = dij_res.first[node_to_insert];
                }
            }

          res.emplace_back(std::move(info));
        }

      return res;
    };


    [[nodiscard]] virtual Output_Type
    start(std::size_t K, dijkstra_result_type const &dij_res) const = 0;

    /// The "general" structure of the Eppstein algorithms. It will construct the shortest paths
    /// \param K The number of shortest paths
    /// \param dij_res The result of the dijkstra algorithm
    /// \param sidetrack_distances The sidetrack distances of every edge
    /// \param h_g The h_g map
    /// \param h_out The h_out map
    /// \param callback_fun A callback function called during the loop used to find the shortest paths
    /// \return The (implicit) shortest paths
    algo_output
    general_algo_eppstein(std::size_t                       K,
                          dijkstra_result_type const       &dij_res,
                          internal_weight_collection const &sidetrack_distances,
                          H_g_collection                   &h_g,
                          H_out_collection                 &h_out,
                          callback_function const          &callback_fun = nullptr) const
    {
      auto const &root   = base::root;
      auto constexpr inf = std::numeric_limits<node_id_type>::max();

      auto const &[successors, shortest_distance] = dij_res;

      // If the shortest path doesn't exist, then we can return an empty vector
      if (shortest_distance[root] == std::numeric_limits<Weight_Type>::max())
        return {};

      // Start with the shortest path
      std::conditional_t<Only_Distance, std::vector<Weight_Type>, std::vector<implicit_path_info>> res;

      if constexpr (Only_Distance)
        {
          res.push_back({shortest_distance[root]});
        }
      else
        {
          res.push_back(implicit_path_info(std::vector<sidetrack>(), shortest_distance[root]));
        }

      // Find the first sidetrack edge
      typename H_g_collection::const_iterator h_g_it = h_g.find(root);

      if (h_g_it == h_g.end() || h_g_it->second.empty())
        return res;

      auto const &first_side_track = extract_first_sidetrack_edge(h_g_it);

      res.reserve(K);

      // Collection of "final" implicit paths to be added to res
      Heap<implicit_path_info, std::greater<>> Q;

      Q.reserve(K);

      // First deviatory path
      Q.emplace(std::vector{first_side_track}, first_side_track.delta_weight + shortest_distance[root]);

#if LOCAL
      double q_interaction1 = .0, q_interaction2 = .0, q_interaction3 = .0, search = .0;
#endif

      std::size_t k = 2;
      // Loop through Q until either Q is empty or the number of paths found is K
      for (; k <= K && !Q.empty(); ++k)
        {
#ifdef LOCAL
          Chrono crono;
          crono.start();
#endif

          auto SK = Q.pop_head();

          if constexpr (Only_Distance)
            {
              res.emplace_back(SK.length);
            }
          else
            {
              res.emplace_back(SK);
            }

#ifdef LOCAL
          crono.stop();
          q_interaction1 += (crono.wallTime() / 1000. / 1000.);

          crono.start();
#endif

          auto const &[current_h_g, current_location, _e_weight] = SK.sidetracks.back();

          auto const &e_h_out            = current_h_g->second.get_elem(current_location.first);
          auto const &[e_edge, e_weight] = current_location.second == std::numeric_limits<node_id_type>::max() ?
                                             e_h_out->second.get_elem(0) :
                                             e_h_out->second.get_elem(current_location.second);

#ifdef LOCAL
          crono.stop();
          search += (crono.wallTime() / 1000. / 1000.);
#endif

          // "Helper" function that can be called if needed
          if (callback_fun != nullptr)
            h_g_it = callback_fun(h_g, h_out, sidetrack_distances, successors, e_edge.second);
          else
            h_g_it = h_g.find(e_edge.second);

#if LOCAL
          crono.start();
#endif

          if (h_g_it != h_g.end() && !h_g_it->second.empty())
            {
              // Extract the first sidetrack edge, if it exists
              auto const &f = extract_first_sidetrack_edge(h_g_it);

              auto mod_sk = SK;
              mod_sk.sidetracks.emplace_back(f);
              mod_sk.length += f.delta_weight;
              Q.emplace(std::move(mod_sk));
            }

#ifdef LOCAL
          crono.stop();
          q_interaction2 += (crono.wallTime() / 1000. / 1000.);
          crono.start();
#endif

          auto const alternatives = get_alternatives(current_h_g, current_location);

#ifdef LOCAL
          crono.stop();
          search += (crono.wallTime() / 1000. / 1000.);
#endif

          if (!alternatives.empty())
            {
              SK.sidetracks.pop_back();

              for (auto const &sidetrack_edge : alternatives)
                {
#if LOCAL
                  crono.start();
#endif

                  auto mod_sk = SK;
                  mod_sk.sidetracks.emplace_back(sidetrack_edge);
                  mod_sk.length += (sidetrack_edge.delta_weight - e_weight);

                  Q.emplace(std::move(mod_sk));

#ifdef LOCAL
                  crono.stop();
                  q_interaction3 += (crono.wallTime() / 1000. / 1000.);
#endif
                }
            }
        }

#if LOCAL
      std::cout << "Q management 1: " << q_interaction1 << " s" << std::endl
                << "Q management 2: " << q_interaction2 << " s" << std::endl
                << "Q management 3: " << q_interaction3 << " s" << std::endl
                << "Heap search: " << search << std::endl
                << "K: " << --k << std::endl;
#endif

      return res;
    }

  public:
    /// Applies a K-shortest path algorithm to find the k-shortest paths on the given graph (from the first node to
    /// the last one)
    /// \param K The number of shortest paths to find
    /// \return The shortest paths
    [[nodiscard]] Output_Type
    compute(std::size_t K) const override
    {
      auto const &graph = base::graph;
      auto const &root  = base::root;
      auto const &sink  = base::sink;

      if (graph.empty() || K == 0)
        return {};

#if LOCAL
      Chrono crono;
      crono.start();
#endif

      auto const dij_res = Shortest_path_finder::dijkstra(this->graph.reverse(), sink); // time:

#if LOCAL
      crono.stop();
      std::cout << "Dijkstra time: " << crono.wallTime() / 1000. / 1000. << " s" << std::endl;
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

      return start(K, dij_res);
    };

    explicit basic_KEppstein(Weighted_Graph<Graph_type> const &g, std::size_t root, std::size_t sink)
      : base(g, root, sink){};

    explicit basic_KEppstein(Graph_type const &g, std::size_t root, std::size_t sink)
      : base(g, root, sink){};

    virtual ~basic_KEppstein() = default;
  };
} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_BASIC_KEPPSTEIN_H
