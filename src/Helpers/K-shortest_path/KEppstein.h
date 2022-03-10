//
// Created by faccus on 01/11/21.
//

#ifndef NETWORK_BUTCHER_KEPPSTEIN_H
#define NETWORK_BUTCHER_KEPPSTEIN_H

#include "KFinder.h"
#include "../Traits/Heap_traits.h"

template <class Graph_type>
class KFinder_Eppstein : public KFinder<Graph_type>
{
public:
  using base          = KFinder<Graph_type>;
  using base_shortest = Shortest_path_finder<Graph_type>;


  /// Applies the Eppstein algorithm to find the k-shortest paths on the given
  /// graph (from the first node to the last one)
  /// \param K The number of shortest paths to find
  /// \return The shortest paths
  [[nodiscard]] std::vector<path_info>
  eppstein(std::size_t K)
  {
    auto const &graph = base_shortest::graph;

    if (graph.get_nodes().empty() || K == 0)
      return {};

    auto const dij_res =
      base_shortest::shortest_path_tree(); // time: ((N+E)log(N)), space: O(N)

    if (K == 1)
      return {base_shortest::shortest_path_finder(dij_res, 0)};


    auto const epp_res = basic_eppstein(K, dij_res);

    return base::helper_eppstein(dij_res, epp_res);
  }

  explicit KFinder_Eppstein(Graph_type const &g)
    : base(g){};

  virtual ~KFinder_Eppstein() = default;

private:

  /// Helper function in the construction of the H_outs
  /// \param successors The list of the successors of every node (the node
  /// following the current one in the shortest path)
  /// \param sidetrack_distances The collection of the sidetrack distances for
  /// all the sidetrack edges
  /// \param real_num_nodes The real number of nodes (that is the number of
  /// nodes taking into account the multiple devices)
  /// \return H_out map
  [[nodiscard]] H_out_collection
  helper_construct_h_out(std::vector<node_id_type> const &successors,
                         weights_collection_type const   &sidetrack_distances,
                         std::size_t const                real_num_nodes) const
  {
    H_out_collection h_out;
    auto const &graph     = base_shortest::graph;

    for (auto i = 0; i < real_num_nodes; ++i) // O(N)
      {
        auto it =
          h_out.insert(h_out.cend(), {i, std::make_shared<H_out<edge_info>>()});
        it->second->heap.id = i;
      }

    for (auto const &edge_pair : sidetrack_distances) // O(E)
      {
        auto const &edge               = edge_pair.first;

        auto const &tail = edge.first;
        auto const &succ = successors[tail];

        if (edge.second != succ)
          {
            auto const &sidetrack_distance = edge_pair.second;
            edge_info tmp(edge, sidetrack_distance);
            auto     &children = h_out[tail]->heap.children;

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
  [[nodiscard]] H_out_collection
  construct_h_out(
    std::vector<node_id_type> const &successors,
    weights_collection_type const   &sidetrack_distances) const // O(N+E*log(N))
  {
    return helper_construct_h_out(successors,
                                  sidetrack_distances,
                                  base_shortest::graph.get_nodes().size());
  }


  /// Given the h_out map, the successors collection and the sidetrack distance,
  /// it will produce the h_g map
  /// \param h_out H_out map
  /// \param successors The list of the successors of every node (the node
  /// following the current one in the shortest path)
  /// \param num_nodes The number of nodes
  /// \return The h_g map
  [[nodiscard]] H_g_collection
  helper_construct_h_g(H_out_collection const &h_out,
                       std::vector<node_id_type> const             &successors,
                       std::size_t const &num_nodes) const // O(N*log(N))
  {
    H_g_collection res;

    auto const &graph = base_shortest::graph;
    auto const &nodes = graph.get_nodes();

    std::vector<std::set<node_id_type>> sp_dependencies;
    sp_dependencies.resize(num_nodes);

    for (auto i = 0; i < num_nodes; ++i)
      {
        auto it       = res.insert(res.cend(), {i, H_g()});
        it->second.id = i;
      }

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
  [[nodiscard]] H_g_collection
  construct_h_g(
    H_out_collection const &h_out,
    std::vector<node_id_type> const &successors) const // O(N*log(N))
  {
    return helper_construct_h_g(h_out,
                                successors,
                                base_shortest::graph.get_nodes().size());
  }


  /// The final function called by the basic_eppstein and basic_eppstein_linear.
  /// It will construct the actual shortest paths
  /// \param K The number of shortest paths
  /// \param dij_res The result of the dijkstra algorithm
  /// \param sidetrack_distances_res The sidetrack distances of every edge
  /// \param h_g The h_g map
  /// \param edge_edges The edge_edges map
  /// \return The (implicit) shortest paths
  std::vector<implicit_path_info>
  base_path_selector_eppstein(
    std::size_t                    K,
    dijkstra_result_type const    &dij_res,
    weights_collection_type const &sidetrack_distances_res,
    H_g_collection const                 &h_g,
    H_out_collection const               &h_out) const
  {
    std::vector<implicit_path_info> res;
    res.push_back({{}, dij_res.second.front()});

    auto const &successors = dij_res.first;
    auto const &graph      = base_shortest::graph;

    auto const first_side_track_res =
      base::extrack_first_sidetrack_edge(0, h_g);
    if (!first_side_track_res.first)
      return res;

    res.reserve(K);

    auto const &first_side_track = first_side_track_res.second;

    edge_edges_type h_out_edge_edges;
    edge_edges_type h_g_edge_edges;

    std::set<implicit_path_info> Q;

    implicit_path_info first_path;
    first_path.sidetracks = {first_side_track.edge};
    first_path.length = first_side_track.delta_weight + dij_res.second.front();

    Q.insert(std::move(first_path));

    auto print_missing_sidetrack_distance = [](edge_type const &e) {
      std::cout << "Error: cannot find proper sidetrack distance for edge ("
                << e.first << ", " << e.second << ")" << std::endl;
    };

    for (int k = 2; k <= K && !Q.empty(); ++k)
      {
        auto SK = *Q.begin();
        Q.erase(Q.begin());
        res.push_back(SK);

        auto const  e      = SK.sidetracks.back();
        auto const &e_edge = *e;

        auto const ot = sidetrack_distances_res.find(e_edge);

        if (ot == sidetrack_distances_res.cend())
          {
            print_missing_sidetrack_distance(e_edge);
            continue;
          }

        auto const f_res =
          base::extrack_first_sidetrack_edge(e_edge.second, h_g);

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

        auto const alternatives = base::get_alternatives(
          h_g.find(h_g_search)->second, h_g_edge_edges, h_out_edge_edges, e);

        if (!alternatives.empty())
          {
            SK.sidetracks.pop_back();

            for (auto const &f : alternatives)
              {
                auto const &f_edge = *f;

                auto ut = sidetrack_distances_res.find(f_edge);

                if (ut == sidetrack_distances_res.cend())
                  {
                    print_missing_sidetrack_distance(f_edge);
                    continue;
                  }

                auto mod_sk = SK;
                mod_sk.sidetracks.push_back(f);
                mod_sk.length += (ut->second - ot->second);

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
  basic_eppstein(std::size_t K, dijkstra_result_type const &dij_res)
  {
    auto const &graph = base_shortest::graph;

    if (graph.get_nodes().empty())
      return {};
    if (K == 1)
      return {{{}, dij_res.second.front()}};

    auto const sidetrack_distances_res =
      base::sidetrack_distances(dij_res.second); // O(E)
    auto const shortest_path =
      base_shortest::shortest_path_finder(dij_res, 0); // O(N)

    auto const &successors          = dij_res.first;
    auto const &shortest_paths_cost = dij_res.second;

    auto const h_out =
      construct_h_out(successors, sidetrack_distances_res); // O(N+E*log(N))

    auto const h_g         = construct_h_g(h_out, successors); // O(N*log(N))

    return base_path_selector_eppstein(
      K, dij_res, sidetrack_distances_res, h_g, h_out);
  }
};


#endif // NETWORK_BUTCHER_KEPPSTEIN_H
