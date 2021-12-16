//
// Created by faccus on 01/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_H
#define NETWORK_BUTCHER_KEPPSTEIN_H

#include "KFinder.h"

template <class T, typename id_content = io_id_type>
class KFinder_Eppstein : public KFinder<T, id_content>
{
public:
  using base          = KFinder<T, id_content>;
  using base_shortest = Shortest_path_finder<T, id_content>;

  using H_out_map = std::map<node_id_type, H_out_pointer>;


  /// Applies the Eppstein algorithm to find the k-shortest paths on the given
  /// graph (from the first node to the last one)
  /// \param weights The weights associated to the different edges
  /// \param K The number of shortest paths to find
  /// \return The shortest paths
  [[nodiscard]] std::vector<path_info>
  eppstein(type_collection_weights const &weights, std::size_t K)
  {
    std::function<type_weight(edge_type const &)> weight_fun =
      [&weights](edge_type const &edge) {
        auto const it = weights.find(edge);
        if (it != weights.cend())
          return it->second;
        return -1.;
      };

    return eppstein(weight_fun, K);
  }


  /// Applies the Eppstein algorithm to find the k-shortest paths on the given
  /// graph (from the first node to the last one)
  /// \param weights The weights associated to the different edges
  /// \param K The number of shortest paths to find
  /// \return The shortest paths
  [[nodiscard]] std::vector<path_info>
  eppstein(std::function<type_weight(edge_type const &)> &weights,
           std::size_t                                    K)
  {
    auto const &graph = base_shortest::graph;

    if (graph.nodes.empty() || K == 0)
      return {};

    auto const dij_res = base_shortest::shortest_path_tree(
      weights); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base_shortest::shortest_path_finder(dij_res, 0)};


    auto const epp_res = basic_eppstein(weights, K, dij_res);

    return base::helper_eppstein(dij_res, epp_res);
  }

  explicit KFinder_Eppstein(Graph<T, id_content> const &g)
    : base(g){};

  virtual ~KFinder_Eppstein() = default;

private:
  /// It will construct a map associating every edge to its children
  /// \param h_gs The h_g map
  /// \return The map associating every edge to its children
  [[nodiscard]] std::map<edge_type, std::set<edge_type>>
  get_edge_edges(std::map<node_id_type, H_g> const &h_gs) const
  {
    std::map<edge_type, std::set<edge_type>> res;

    for (auto it = h_gs.cbegin(); it != h_gs.cend(); ++it) // O(N)
      base::get_internal_edges(res, it->second);
    return res;
  }

  /// Helper function in the construction of the H_outs
  /// \param successors The list of the successors of every node (the node
  /// following the current one in the shortest path)
  /// \param sidetrack_distances The collection of the sidetrack distances for
  /// all the sidetrack edges
  /// \param real_num_nodes The real number of nodes (that is the number of
  /// nodes taking into account the multiple devices)
  /// \return H_out map
  [[nodiscard]] H_out_map
  helper_construct_h_out(std::vector<node_id_type> const &successors,
                         type_collection_weights const   &sidetrack_distances,
                         std::size_t const                real_num_nodes) const
  {
    H_out_map   h_out;
    auto const &graph     = base_shortest::graph;
    auto const  num_nodes = graph.nodes.size();

    for (auto i = 0; i < real_num_nodes; ++i)
      h_out.insert(h_out.cend(), {i, std::make_shared<H_out>()});

    for (auto const &edge_pair : sidetrack_distances) // O(E)
      {
        auto const &edge               = edge_pair.first;
        auto const &sidetrack_distance = edge_pair.second;

        auto const &tail = edge.first;
        auto const &succ = successors[tail];

        if (edge.second != succ)
          {
            edge_info tmp;
            tmp.edge         = edge;
            tmp.delta_weight = sidetrack_distance;
            auto &children   = h_out[tail]->heap.children;

            children.insert(std::move(tmp)); // O(log(N))
          }
      }

    return h_out;
  }

  /// Given the successors collection and the sidetrack distances, it will
  /// construct the h_out map
  /// \param successors The list of the successors of every node (the node
  /// following the current one in the shortest path)
  /// \param sidetrack_distances The collection of the sidetrack distances for
  /// all the sidetrack edges
  /// \return H_out map
  [[nodiscard]] H_out_map
  construct_h_out(
    std::vector<node_id_type> const &successors,
    type_collection_weights const   &sidetrack_distances) const // O(N+E*log(N))
  {
    return helper_construct_h_out(successors,
                                  sidetrack_distances,
                                  base_shortest::graph.nodes.size());
  }


  /// Given the h_out map, the successors collection and the sidetrack distance,
  /// it will produce the h_g map
  /// \param h_out H_out map
  /// \param successors The list of the successors of every node (the node
  /// following the current one in the shortest path)
  /// \param num_nodes The number of nodes
  /// \return The h_g map
  [[nodiscard]] std::map<node_id_type, H_g>
  helper_construct_h_g(std::map<node_id_type, H_out_pointer> const &h_out,
                       std::vector<node_id_type> const             &successors,
                       std::size_t const &num_nodes) const // O(N*log(N))
  {
    std::map<node_id_type, H_g> res;

    auto const &graph = base_shortest::graph;
    auto const &nodes = graph.nodes;


    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(num_nodes);

    for (auto i = 0; i < num_nodes; ++i)
      res.insert(res.cend(), {i, H_g()});

    for (auto i = 0; i < num_nodes; ++i) // O(N)
      {
        auto const &tmp = successors[i];

        if (tmp != i)
          sp_dependencies[tmp].insert(i);
      }

    auto const sink = nodes.size() - 1;

    auto iterator = h_out.find(sink);
    if (iterator != h_out.cend() && !iterator->second->heap.children.empty())
      {
        res[sink].children.emplace(iterator->second); // O(log(N))
      }

    std::queue<node_id_type> queue;

    for (auto i = 0; i < res.size(); ++i)
      if (successors[i] == sink)
        queue.push(i);

    while (!queue.empty()) // O(N)
      {
        auto &deps = sp_dependencies[queue.front()];

        for (auto &n : deps)
          queue.push(n);

        auto &heap_node = res[queue.front()]; // O(log(N))

        iterator = h_out.find(queue.front());
        if (iterator != h_out.cend() &&
            !iterator->second->heap.children.empty())
          heap_node.children.emplace(iterator->second); // O(1)

        auto const &tmo = res[successors[queue.front()]];

        if (!tmo.children.empty())
          heap_node.children.insert(tmo.children.begin(),
                                    tmo.children.end()); // O(1)


        queue.pop();
      }

    return res;
  }

  /// It will produce the map associating every node to its corresponding H_g
  /// map \param h_out The collection of h_outs \param successors The successors
  /// list \return The map associating every node to its corresponding H_g map
  [[nodiscard]] std::map<node_id_type, H_g>
  construct_h_g(
    std::map<node_id_type, H_out_pointer> const &h_out,
    std::vector<node_id_type> const &successors) const // O(N*log(N))
  {
    return helper_construct_h_g(h_out,
                                successors,
                                base_shortest::graph.nodes.size());
  }


  /// The final function called by the basic_eppstein and basic_eppstein_linear.
  /// It will construct the actual shortest paths
  /// \param K The number of shortest paths
  /// \param dij_res The result of the dijkstra algorithm
  /// \param sidetrack_distances_res The sidetrack distances of every edge
  /// \param h_g The h_g map
  /// \param edges_edges The edge_edges map
  /// \return The (implicit) shortest paths
  std::vector<implicit_path_info>
  base_path_selector_eppstein(
    std::size_t                                     K,
    dijkstra_result_type const                     &dij_res,
    type_collection_weights const                  &sidetrack_distances_res,
    std::map<node_id_type, H_g> const              &h_g,
    std::map<edge_type, std::set<edge_type>> const &edges_edges) const
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()});

    auto const &successors = dij_res.first;
    auto const &graph      = base_shortest::graph;

    auto const first_side_track_res =
      base::extrack_first_sidetrack_edge(0, h_g);
    if (!first_side_track_res.first)
      return res;
    auto const &first_side_track = first_side_track_res.second;

    std::set<implicit_path_info> Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.insert(std::move(first_path));

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = *Q.begin();
        Q.erase(Q.begin());
        res.push_back(SK);

        auto const e  = SK.sidetracks.back();
        auto const ot = sidetrack_distances_res.find(e);

        if (ot == sidetrack_distances_res.cend())
          {
            std::cout
              << "Error: cannot find proper sidetrack distance for edge ("
              << e.first << ", " << e.second << ")" << std::endl;
            continue;
          }

        auto const f_res = base::extrack_first_sidetrack_edge(e.second, h_g);

        if (f_res.first)
          {
            auto const &f = f_res.second;

            auto mod_sk = SK;
            mod_sk.sidetracks.push_back(f.edge);
            mod_sk.length += f.delta_weight;
            Q.insert(std::move(mod_sk));
          }

        auto const it = edges_edges.find(e);
        if (it != edges_edges.cend())
          {
            SK.sidetracks.pop_back();

            for (auto const &f : it->second)
              {
                auto ut = sidetrack_distances_res.find(f);

                if (ut == sidetrack_distances_res.cend())
                  {
                    std::cout << "Error: cannot find proper sidetrack distance "
                                 "for edge ("
                              << f.first << ", " << f.second << ")"
                              << std::endl;
                    continue;
                  }

                auto mod_sk = SK;
                mod_sk.sidetracks.push_back(f);
                mod_sk.length += (ut->second - ot->second);

                if (!SK.sidetracks.empty())
                  {
                    auto n =
                      mod_sk.sidetracks[mod_sk.sidetracks.size() - 2].second;
                    while (n != graph.nodes.size() - 1 &&
                           n != mod_sk.sidetracks.back().first)
                      {
                        n = successors[n];
                      }

                    if (n != mod_sk.sidetracks.back().first)
                      continue;
                  }

                Q.insert(std::move(mod_sk));
              }
          }
      }

    return res;
  }


  /// The basic function for the Eppstein algorithm
  /// \param weights The weights of the edges
  /// \param K The number of shortest paths
  /// \param dij_res The result of dijkstra
  /// \return The (implicit) k shortest paths
  [[nodiscard]] std::vector<implicit_path_info>
  basic_eppstein(std::function<type_weight(edge_type const &)> &weights,
                 std::size_t                                    K,
                 dijkstra_result_type const                    &dij_res)
  {
    auto const &graph = base_shortest::graph;

    if (graph.nodes.empty())
      return {};
    if (K == 1)
      return {{{}, dij_res.second.front()}};

    auto const sidetrack_distances_res =
      base::sidetrack_distances(weights, dij_res.second); // O(E)
    auto const shortest_path =
      base_shortest::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out =
      construct_h_out(successors, sidetrack_distances_res); // O(N+E*log(N))

    auto const h_g         = construct_h_g(h_out, successors); // O(N*log(N))
    auto       edges_edges = get_edge_edges(h_g);

    return base_path_selector_eppstein(
      K, dij_res, sidetrack_distances_res, h_g, edges_edges);
  }

};


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
