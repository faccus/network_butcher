//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KFINDER_H
#define NETWORK_BUTCHER_KFINDER_H

#include "Shortest_path_finder.h"
template <class T, typename id_content = io_id_type>
class KFinder : public Shortest_path_finder<T, id_content>
{
public:
  using base = Shortest_path_finder<T, id_content>;

  explicit KFinder(Graph<T, id_content> const &g)
    : base(g){};

  virtual ~KFinder() = default;

protected:
  /// It extracts the first sidetrack associated to the given node
  /// \param j The index of the node
  /// \param h_g The h_g map
  /// \return The pair: the operation completed successfully and the
  /// corresponding sidetrack edge
  [[nodiscard]] std::pair<bool, edge_info>
  extrack_first_sidetrack_edge(node_id_type const                &j,
                               std::map<node_id_type, H_g> const &h_g) const
  {
    auto const it = h_g.find(j);
    if (it == h_g.cend() || it->second.children.empty() ||
        (*it->second.children.begin())->heap.children.empty())
      {
        return {false, {{-1, -1}, std::numeric_limits<type_weight>::max()}};
      }

    return {true, *(*it->second.children.begin())->heap.children.begin()};
  }

  /// Computes the sidetrack distances for all the different sidetrack edges
  /// \param weights The weight map (for the edges)
  /// \param distances_from_sink The shortest distance from the given node to
  /// the sink (the last node of the graph)
  /// \return The collection of sidetrack distances for the different edges
  [[nodiscard]] type_collection_weights
  sidetrack_distances(std::function<type_weight(edge_type const &)> &weights,
                      std::vector<type_weight> const &distances_from_sink) const
  {
    type_collection_weights res;

    auto const &graph     = base::graph;
    auto const  num_nodes = graph.nodes.size();

    for (std::size_t tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : graph.dependencies[tail].second)
        {
          auto const edge = std::make_pair(tail, head);

          res.insert(res.cend(),
                     {edge,
                      weights(edge) + distances_from_sink[head] -
                        distances_from_sink[tail]}); // O(1)
        }

    return res;
  }


  /// It will update edge_edges with the parent-child relationships in h_out
  /// \param edge_edges The map of childrens of the given edge
  /// \param h_out H_out of a given node
  void
  get_internal_edges(std::map<edge_type, std::set<edge_type>> &edge_edges,
                     H_out_pointer const                      &h_out) const
  {
    std::size_t                                      j = 0;
    std::vector<std::set<edge_info>::const_iterator> previous_steps;
    previous_steps.reserve(h_out->heap.children.size());

    for (auto it = h_out->heap.children.cbegin();
         it != h_out->heap.children.cend();
         ++it, ++j)
      {
        previous_steps.push_back(it);

        std::size_t parent = j / 2;
        if (parent != j)
          {
            auto const &parent_edge  = previous_steps[parent]->edge;
            auto const &current_edge = it->edge;

            edge_edges[parent_edge].insert(current_edge);
          }
      }
  }

  /// It will update edge_edges with the parent-child relationships in h_g
  /// \param edge_edges The map of childrens of the given edge
  /// \param h_g H_g of a given node
  /// \param include_h_outs Calls get_internal_edges for all the encountered
  /// H_outs in H_g
  void
  get_internal_edges(std::map<edge_type, std::set<edge_type>> &edge_edges,
                     H_g const                                &h_g,
                     bool include_h_outs = true) const
  {
    std::size_t                                          j = 0;
    std::vector<std::set<H_out_pointer>::const_iterator> previous_steps;
    previous_steps.reserve(h_g.children.size());
    const auto &graph = base::graph;

    for (auto it = h_g.children.cbegin(); it != h_g.children.cend();
         ++it, ++j) // O(N)
      {
        previous_steps.push_back(it);

        if (include_h_outs)
          get_internal_edges(edge_edges, *it); // O(N)

        std::size_t parent = (j - 1) / 2;
        if (j > 0 && parent != j)
          {
            auto const &parent_edge =
              (*previous_steps[parent])->heap.children.cbegin()->edge;
            auto const &child_edge = (*it)->heap.children.cbegin()->edge;

            edge_edges[parent_edge].insert(child_edge);
          }
      }
  }

  /// Helper function for the Eppstein algorithm. It converts a vector of
  /// implicit paths to a vector of explicit paths
  /// \param dij_res The result of the Dijkstra result
  /// \param epp_res The result of basic_eppstein or basic_eppstein_linear
  /// \return The shortest paths
  [[nodiscard]] std::vector<path_info>
  helper_eppstein(dijkstra_result_type const            &dij_res,
                  std::vector<implicit_path_info> const &epp_res)
  {
    std::vector<path_info> res;
    auto const            &graph = base::graph;

    for (auto implicit_path = epp_res.cbegin(); implicit_path != epp_res.cend();
         ++implicit_path)
      {
        path_info info;
        info.length = implicit_path->length;
        info.path.reserve(graph.nodes.size());

        auto const &sidetracks = implicit_path->sidetracks;

        auto        it             = sidetracks.cbegin();
        std::size_t node_to_insert = 0;

        while (node_to_insert != graph.nodes.back().get_id())
          {
            info.path.push_back(node_to_insert);
            if (it != sidetracks.cend() && it->first == node_to_insert)
              {
                node_to_insert = it->second;
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
};

#endif // NETWORK_BUTCHER_KFINDER_H
