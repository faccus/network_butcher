//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KFINDER_H
#define NETWORK_BUTCHER_KFINDER_H

#include "Heap_traits.h"
#include "Shortest_path_finder.h"

namespace network_butcher_kfinder
{
  /// A (pure) virtual class to find the K shortest path for a given path
  /// \tparam Graph_type The graph type
  template <class Graph_type>
  class KFinder
  {
  protected:
    Graph_type const &graph;

    using callback_function_helper_eppstein = std::function<void(H_g_collection &,
                                                                 H_out_collection &,
                                                                 weights_collection_type const &,
                                                                 std::vector<node_id_type> const &,
                                                                 node_id_type)>;

    using callback_function_helper_ptr_eppstein = std::unique_ptr<callback_function_helper_eppstein>;

    /// It extracts the first sidetrack associated to the given node
    /// \param j The index of the node
    /// \param h_g The h_g map
    /// \return The pair: the operation completed successfully and the
    /// corresponding sidetrack edge
    [[nodiscard]] std::pair<bool, edge_info>
    extrack_first_sidetrack_edge(node_id_type const &j, H_g_collection const &h_g) const;

    /// Computes the sidetrack distances for all the different sidetrack edges
    /// \param weights The weight map (for the edges)
    /// \param distances_from_sink The shortest distance from the given node to
    /// the sink (the last node of the graph)
    /// \return The collection of sidetrack distances for the different edges
    [[nodiscard]] weights_collection_type
    sidetrack_distances(std::vector<weight_type> const &distances_from_sink) const;


    /// It will return edge_edges with the parent-child relationships in h_out
    /// \param h_out H_out of a given node
    /// \return edge_edges The map of childrens for a given edge in h_out
    [[nodiscard]] h_edge_edges_type
    get_internal_edges(H_out_pointer const &h_out) const;


    edge_sequence
    get_alternatives(H_g const          &h_g,
                     edge_edges_type    &h_g_edge_edges,
                     edge_edges_type    &h_out_edge_edges,
                     edge_pointer const &edge) const;


    /// Helper function for the Eppstein algorithm. It converts a vector of implicit paths to a vector of explicit paths
    /// \param dij_res The result of the Dijkstra result
    /// \param epp_res The result of basic_eppstein or basic_eppstein_linear
    /// \return The shortest paths
    [[nodiscard]] std::vector<path_info>
    helper_eppstein(dijkstra_result_type const &dij_res, std::vector<implicit_path_info> const &epp_res) const;

    /// The final function called by the basic_eppstein and
    /// basic_eppstein_linear. It will construct the actual shortest paths
    /// \param K The number of shortest paths
    /// \param dij_res The result of the dijkstra algorithm
    /// \param sidetrack_distances_res The sidetrack distances of every edge
    /// \param h_g The h_g map
    /// \param edge_edges The edge_edges map
    /// \return The (implicit) shortest paths
    std::vector<implicit_path_info>
    general_algo_eppstein(std::size_t                                  K,
                          dijkstra_result_type const                  &dij_res,
                          weights_collection_type const               &sidetrack_distances_res,
                          H_g_collection                              &h_g,
                          H_out_collection                            &h_out,
                          callback_function_helper_ptr_eppstein const &callback_fun_ptr = nullptr) const;


  public:
    /// Applies a K-shortest path algorithm to find the k-shortest paths on the
    /// given graph (from the first node to the last one)
    /// \param K The number of shortest paths to find
    /// \return The shortest paths
    [[nodiscard]] virtual std::vector<path_info>
    compute(std::size_t K) const = 0;

    explicit KFinder(Graph_type const &g)
      : graph(g){};

    virtual ~KFinder() = default;
  };

  template <class Graph_type>
  std::pair<bool, edge_info>
  KFinder<Graph_type>::extrack_first_sidetrack_edge(const node_id_type &j, const H_g_collection &h_g) const
  {
    auto const it = h_g.find(j);
    if (it == h_g.cend() || it->second.children.empty() || (*it->second.children.begin())->heap.children.empty())
      {
        return {false, {{-1, -1}, std::numeric_limits<weight_type>::max()}};
      }

    return {true, *(*it->second.children.begin())->heap.children.begin()};
  }

  template <class Graph_type>
  weights_collection_type
  KFinder<Graph_type>::sidetrack_distances(const std::vector<weight_type> &distances_from_sink) const
  {
    weights_collection_type res;

    auto const num_nodes = graph.size();

    for (std::size_t tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : graph.get_dependencies()[tail].second)
        {
          auto const edge = std::make_pair(tail, head);

          res.insert(res.cend(),
                     {edge, graph.get_weigth(edge) + distances_from_sink[head] - distances_from_sink[tail]}); // O(1)
        }

    return res;
  }

  template <class Graph_type>
  h_edge_edges_type
  KFinder<Graph_type>::get_internal_edges(const H_out_pointer &h_out) const
  {
    h_edge_edges_type edge_edges;

    std::size_t                                                   j = 0;
    std::vector<H_out<edge_info>::container_type::const_iterator> previous_steps;
    previous_steps.reserve(h_out->heap.children.size());

    // Basic loop to find the children of the H_out heap (a node connected to a binary heap):
    //
    //             0
    //             |
    //             1
    //           /   \
    //          2     3
    //         / \   / \
    //        4  5  6   7
    for (auto it = h_out->heap.children.cbegin(); it != h_out->heap.children.cend(); ++it, ++j)
      {
        previous_steps.push_back(it);

        std::size_t parent = j / 2;
        if (parent != j)
          {
            auto const &parent_edge  = previous_steps[parent]->edge;
            auto const &current_edge = it->edge;

            edge_edges[parent_edge].push_back(current_edge);
          }
      }

    return edge_edges;
  }

  template <class Graph_type>
  edge_sequence
  KFinder<Graph_type>::get_alternatives(const H_g          &h_g,
                                        edge_edges_type    &h_g_edge_edges,
                                        edge_edges_type    &h_out_edge_edges,
                                        const edge_pointer &edge) const
  {
    {
      auto const tmp_it = h_g_edge_edges.find(h_g.id);

      if (tmp_it != h_g_edge_edges.cend())
        return (tmp_it->second)[edge];
    }

    auto                                            &h_g_map = h_g_edge_edges[h_g.id];
    std::size_t                                      j       = 0;
    std::vector<H_g::container_type::const_iterator> previous_steps;
    previous_steps.reserve(h_g.children.size());


    // Basic loop to find the children of the H_g heap (a binary heap):
    //            H_g
    //             0
    //           /   \        "linked" H_out
    //          1     2 ------------ 2
    //         / \   / \             |
    //        3  4  5   6            10
    for (auto it = h_g.children.cbegin(); it != h_g.children.cend(); ++it, ++j)
      {
        previous_steps.push_back(it);

        auto const associated_h_out    = (*it)->heap.id;

        auto       h_out_edge_edges_it = h_out_edge_edges.find(associated_h_out);

        // If get_internal_edges was not called for associated_h_out, call it
        if (h_out_edge_edges_it == h_out_edge_edges.cend())
          {
            auto tmp            = h_out_edge_edges.insert({associated_h_out, get_internal_edges(*it)});
            h_out_edge_edges_it = tmp.first;
          }

        // Add in the H_g map all the H_out dependencies for associated_h_out
        h_g_map.insert(h_out_edge_edges_it->second.cbegin(), h_out_edge_edges_it->second.cend());

        std::size_t parent = (j - 1) / 2;
        if (parent != j && j > 0)
          {
            auto const &parent_edge  = (*previous_steps[parent])->heap.children.begin()->edge;
            auto const &current_edge = (*it)->heap.children.begin()->edge;

            h_g_map[parent_edge].push_back(current_edge);
          }
      }

    return h_g_map[edge];
  }

  template <class Graph_type>
  std::vector<path_info>
  KFinder<Graph_type>::helper_eppstein(const dijkstra_result_type            &dij_res,
                                       const std::vector<implicit_path_info> &epp_res) const
  {
    std::vector<path_info> res;
    res.reserve(epp_res.size());

    // Basically, we start from the specified node and go along the shortest path until we meet a sidetrack edge
    // contained in the implicit path. In that case, we add the sidetrack edge and proceed along the "new" shortest
    // path until either the "sink" node is reached or another sidetrack edge is met
    for (auto implicit_path = epp_res.cbegin(); implicit_path != epp_res.cend(); ++implicit_path)
      {
        auto const &nodes = graph.get_nodes();

        path_info info;
        info.length = implicit_path->length;
        info.path.reserve(graph.size());

        auto const &sidetracks = implicit_path->sidetracks;

        auto        it             = sidetracks.cbegin();
        std::size_t node_to_insert = 0;

        while (node_to_insert != nodes.back().get_id())
          {
            info.path.push_back(node_to_insert);
            if (it != sidetracks.cend() && (*it)->first == node_to_insert)
              {
                node_to_insert = (*it)->second;
                ++it;
              }
            else
              node_to_insert = dij_res.first[node_to_insert];
          }

        info.path.push_back(node_to_insert);
        res.emplace_back(std::move(info));
      }

    return res;
  }

  template <class Graph_type>
  std::vector<implicit_path_info>
  KFinder<Graph_type>::general_algo_eppstein(
    std::size_t                                           K,
    const dijkstra_result_type                           &dij_res,
    const weights_collection_type                        &sidetrack_distances_res,
    H_g_collection                                       &h_g,
    H_out_collection                                     &h_out,
    const KFinder::callback_function_helper_ptr_eppstein &callback_fun_ptr) const
  {
    auto const &successors = dij_res.first;

    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()});

    // Find the first sidetrack edge
    auto const first_side_track_res = extrack_first_sidetrack_edge(0, h_g);
    if (!first_side_track_res.first)
      return res;
    res.reserve(K);

    // Edges in the graph D(G)
    edge_edges_type h_out_edge_edges;
    edge_edges_type h_g_edge_edges;

    // Collection of "final" implicit paths
    std::set<implicit_path_info> Q;

    implicit_path_info first_path;
    auto const        &first_side_track = first_side_track_res.second;
    first_path.sidetracks               = {first_side_track.edge};
    first_path.length                   = first_side_track.delta_weight + dij_res.second.front();

    // First deviatory path
    Q.insert(std::move(first_path));

    auto print_missing_sidetrack_distance = [](edge_type const &e) {
      std::cout << "Error: cannot find proper sidetrack distance for edge (" << e.first << ", " << e.second << ")"
                << std::endl;
    };

    // Loop through Q until either Q is empty or the number of paths found is K
    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = *Q.begin();
        Q.erase(Q.begin());
        res.push_back(SK);

        auto const  e      = SK.sidetracks.back();
        auto const &e_edge = *e;

        // Find sidetrack weight
        auto const e_sidetrack_edge_it = sidetrack_distances_res.find(e_edge);

        if (e_sidetrack_edge_it == sidetrack_distances_res.cend())
          {
            print_missing_sidetrack_distance(e_edge);
            continue;
          }

        // "Helper" function that can be called if needed
        if (callback_fun_ptr != nullptr)
          (*callback_fun_ptr)(h_g, h_out, sidetrack_distances_res, successors, e_edge.second);

        // Extract the first sidetrack edge, if it exists
        auto const f_res = extrack_first_sidetrack_edge(e_edge.second, h_g);

        if (f_res.first)
          {
            auto const &f = f_res.second;

            auto mod_sk = SK;
            mod_sk.sidetracks.push_back(f.edge);
            mod_sk.length += f.delta_weight;
            Q.insert(std::move(mod_sk));
          }

        node_id_type h_g_search;
        if (SK.sidetracks.size() == 1)
          h_g_search = 0;
        else
          {
            auto const tmp_it = ++SK.sidetracks.crbegin();
            h_g_search        = (*tmp_it)->second;
          }

        auto const alternatives = get_alternatives(h_g.find(h_g_search)->second, h_g_edge_edges, h_out_edge_edges, e);

        if (!alternatives.empty())
          {
            SK.sidetracks.pop_back();

            for (auto const &f : alternatives)
              {
                auto const &f_edge = *f;

                auto f_sidetrack_weight_it = sidetrack_distances_res.find(f_edge);

                if (f_sidetrack_weight_it == sidetrack_distances_res.cend())
                  {
                    print_missing_sidetrack_distance(f_edge);
                    continue;
                  }

                auto mod_sk = SK;
                mod_sk.sidetracks.push_back(f);
                mod_sk.length += (f_sidetrack_weight_it->second - e_sidetrack_edge_it->second);

                Q.insert(std::move(mod_sk));
              }
          }
      }

    return res;
  }

} // namespace network_butcher_kfinder


#endif // NETWORK_BUTCHER_KFINDER_H
