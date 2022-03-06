//
// Created by faccus on 21/11/21.
//

#ifndef NETWORK_BUTCHER_KFINDER_H
#define NETWORK_BUTCHER_KFINDER_H

#include "Shortest_path_finder.h"

#include <forward_list>


template <class Graph_type>
class KFinder : public Shortest_path_finder<Graph_type>
{
public:
  using base = Shortest_path_finder<Graph_type>;

  explicit KFinder(Graph_type const &g)
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
        return {false, {{-1, -1}, std::numeric_limits<weight_type>::max()}};
      }

    return {true, *(*it->second.children.begin())->heap.children.begin()};
  }

  /// Computes the sidetrack distances for all the different sidetrack edges
  /// \param weights The weight map (for the edges)
  /// \param distances_from_sink The shortest distance from the given node to
  /// the sink (the last node of the graph)
  /// \return The collection of sidetrack distances for the different edges
  [[nodiscard]] weights_collection_type
  sidetrack_distances(std::vector<weight_type> const &distances_from_sink) const
  {
    weights_collection_type res;

    auto const &graph     = base::graph;
    auto const  num_nodes = graph.get_nodes().size();

    for (std::size_t tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : graph.get_dependencies()[tail].second)
        {
          auto const edge = std::make_pair(tail, head);

          res.insert(res.cend(),
                     {edge,
                      graph.get_weigth(edge) + distances_from_sink[head] -
                        distances_from_sink[tail]}); // O(1)
        }

    return res;
  }


  /// It will return edge_edges with the parent-child relationships in h_out
  /// \param h_out H_out of a given node
  /// \return edge_edges The map of childrens for a given edge in h_out
  [[nodiscard]] std::map<edge_pointer, std::forward_list<edge_pointer>>
  get_internal_edges(H_out_pointer const &h_out) const
  {
    std::map<edge_pointer, std::forward_list<edge_pointer>> edge_edges;

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

            edge_edges[parent_edge].push_front(current_edge);
          }
      }

    return edge_edges;
  }


  std::forward_list<edge_pointer>
  get_alternatives(
    H_g const &h_g,
    std::map<node_id_type,
             std::map<edge_pointer, std::forward_list<edge_pointer>>>
      &h_g_edge_edges,
    std::map<node_id_type,
             std::map<edge_pointer, std::forward_list<edge_pointer>>>
                       &h_out_edge_edges,
    edge_pointer const &edge) const
  {
    {
      auto const tmp_it = h_g_edge_edges.find(h_g.id);

      if (tmp_it != h_g_edge_edges.cend())
        return (tmp_it->second)[edge];
    }

    auto &h_g_map = h_g_edge_edges[h_g.id];
    std::size_t                                          j = 0;
    std::vector<std::set<H_out_pointer>::const_iterator> previous_steps;
    previous_steps.reserve(h_g.children.size());

    for (auto it = h_g.children.cbegin(); it != h_g.children.cend(); ++it, ++j)
      {
        previous_steps.push_back(it);

        auto const associated_h_out = (*it)->heap.id;
        auto h_out_edge_edges_it    = h_out_edge_edges.find(associated_h_out);

        if (h_out_edge_edges_it == h_out_edge_edges.cend())
          {
            auto tmp = h_out_edge_edges.insert(
              {associated_h_out, get_internal_edges(*it)});
            h_out_edge_edges_it = tmp.first;
          }

        h_g_map.insert(h_out_edge_edges_it->second.cbegin(),
                       h_out_edge_edges_it->second.cend());

        std::size_t parent = (j - 1) / 2;
        if (parent != j && j > 0)
          {
            auto const &parent_edge =
              (*previous_steps[parent])->heap.children.begin()->edge;
            auto const &current_edge = (*it)->heap.children.begin()->edge;

            h_g_map[parent_edge].push_front(current_edge);
          }
      }

    return h_g_map[edge];
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
        auto const &nodes = graph.get_nodes();

        path_info info;
        info.length = implicit_path->length;
        info.path.reserve(graph.get_nodes().size());

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
};

#endif // NETWORK_BUTCHER_KFINDER_H
