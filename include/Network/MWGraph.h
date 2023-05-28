//
// Created by root on 15/03/22.
//

#ifndef NETWORK_BUTCHER_MWGRAPH_H
#define NETWORK_BUTCHER_MWGRAPH_H

#include "Graph.h"


namespace network_butcher::types
{
  /// A custom graph class. It contains a single graph and multiple weight maps. Technically, it can be viewed
  /// as a collection of graphs with the same structure, but different weight maps.
  /// \tparam T Type of the content of the nodes
  template <bool Parallel_Edges, typename t_Node_Type = Node, typename t_weight_type = weight_type>
  class MWGraph : public Graph<t_Node_Type>
  {
  private:
    using Parent_Type = Graph<t_Node_Type>;

  public:
    using Dependencies_Type    = Parent_Type::Neighbours_Type;
    using Node_Type            = Parent_Type::Node_Type;
    using Node_Collection_Type = Parent_Type::Node_Collection_Type;

    using Weight_Type = t_weight_type;

    using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<Weight_Type>, Weight_Type>;
    using Weight_Collection_Type =
      std::conditional_t<Parallel_Edges, std::multimap<edge_type, Weight_Type>, std::map<edge_type, Weight_Type>>;

  private:
    std::vector<Weight_Collection_Type> weigth_map;

  public:
    template <typename A, typename B>
    explicit MWGraph(std::size_t num_maps, A &&v, B &&dep)
      : Parent_Type(std::forward<A>(v), std::forward<B>(dep))
      , weigth_map{}
    {
      if (num_maps == 0)
        {
          throw std::runtime_error("MWGraph: the number of maps must be greater than 0");
        }

      weigth_map.resize(num_maps);
    }


    /// Checks if the given edge has a weight on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] bool
    check_weight(std::size_t device, edge_type const &edge) const
    {
      return weigth_map[device].contains(edge);
    }


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] auto
    get_weight(std::size_t device, edge_type const &edge) const
    {
      auto const      &map = weigth_map[device];
      auto             it  = map.find(edge);
      Edge_Weight_Type res;

      if (it == map.cend())
        {
          if (Parent_Type::check_edge(edge))
            {
              throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) +
                                       " was not associated with any weight of device " + std::to_string(device));
            }
          else
            {
              throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
            }
        }

      if constexpr (Parallel_Edges)
        {
          for (; it != map.cend() && it->first == edge; ++it)
            res.insert(it->second);
        }
      else
        {
          res = it->second;
        }

      return res;
    }


    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(std::size_t device, edge_type const &edge, t_weight_type const &weight)
    {
      if (!Parent_Type::check_edge(edge))
        {
          throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
        }

      if constexpr (Parallel_Edges)
        {
          weigth_map[device].emplace(edge, weight);
        }
      else
        {
          weigth_map[device][edge] = weight;
        }
    }

    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weights The weight
    void
    set_weight(std::size_t device, edge_type const &edge, Edge_Weight_Type weights)
      requires Parallel_Edges
    {
      if (Parent_Type::check_edge(edge))
        {
          for (auto const &weight : weights)
            weigth_map[device].emplace(edge, weight);
        }
      else
        {
          throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
        }
    }


    /// Gets the number of devices
    /// \return Number of devices
    [[nodiscard]] std::size_t
    get_num_devices() const
    {
      return weigth_map.size();
    }

    /// Simple helper function that will print the graph
    /// \return The graph description
    [[nodiscard]] virtual std::string
    print_graph() const
    {
      std::stringstream builder;
      builder << "In Out Weight_Map_Id Weight" << std::endl;

      std::set<std::pair<node_id_type, node_id_type>> recorded_edges;
      for (auto const &node : Parent_Type::nodes)
        {
          for (auto const &out : Parent_Type::get_output_nodes(node.get_id()))
            {
              recorded_edges.emplace_hint(recorded_edges.end(), std::make_pair(node.get_id(), out));
              std::string base_tmp = std::to_string(node.get_id()) + " " + std::to_string(out) + " ";
              for (std::size_t i = 0; i < weigth_map.size(); ++i)
                {
                  auto const &map = weigth_map[i];

                  if constexpr (Parallel_Edges)
                    {
                      std::string new_base = base_tmp + std::to_string(i) + " ";
                      auto        it       = map.find(std::make_pair(node.get_id(), out));
                      if (it != map.cend())
                        {
                          for (auto const &w : it->second)
                            {
                              builder << new_base << Utilities::custom_to_string(w) << std::endl;
                            }
                        }
                      else
                        {
                          builder << new_base << "$" << std::endl;
                        }
                    }
                  else
                    {
                      builder << base_tmp << std::to_string(i) << " ";

                      auto it = map.find(std::make_pair(node.get_id(), out));
                      if (it != map.cend())
                        {
                          builder << Utilities::custom_to_string(it->second);
                        }
                      else
                        {
                          builder << "$";
                        }

                      builder << std::endl;
                    }
                }
            }
        }

      return builder.str();
    }


    ~MWGraph() override = default;
  };

  /// A custom graph class. It contains a single graph and multiple weight maps. Technically, it can be viewed
  /// as a collection of graphs with the same structure, but different weight maps.
  /// \tparam T Type of the content of the nodes
  template <bool Parallel_Edges, typename T>
  class MWGraph<Parallel_Edges, CNode<Content<T>>, weight_type> : public Graph<CNode<Content<T>>>
  {
  private:
    using Parent_Type = Graph<CNode<Content<T>>>;

  public:
    using Dependencies_Type    = Parent_Type::Neighbours_Type;
    using Node_Type            = Parent_Type::Node_Type;
    using Node_Collection_Type = Parent_Type::Node_Collection_Type;

    using Weight_Type = weight_type;

    using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<weight_type>, weight_type>;
    using Weight_Collection_Type =
      std::conditional_t<Parallel_Edges, std::multimap<edge_type, weight_type>, std::map<edge_type, weight_type>>;

  protected:
    std::vector<Weight_Collection_Type> weigth_map;

  public:
    template <typename A, typename B>
    explicit MWGraph(std::size_t num_maps, A &&v, B &&dep)
      : Parent_Type(std::forward<A>(v), std::forward<B>(dep))
      , weigth_map{}
    {
      if (num_maps == 0)
        {
          throw std::runtime_error("MWGraph: the number of maps must be greater than 0");
        }
      weigth_map.resize(num_maps);
    }

    template <typename A>
    explicit MWGraph(std::size_t num_maps, A &&v)
      : Parent_Type(std::forward<A>(v))
      , weigth_map{}
    {
      if (num_maps == 0)
        {
          throw std::runtime_error("MWGraph: the number of maps must be greater than 0");
        }
      weigth_map.resize(num_maps);
    }


    /// Checks if the given edge has a weight on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] bool
    check_weight(std::size_t device, edge_type const &edge) const
    {
      return weigth_map[device].contains(edge);
    }


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] Edge_Weight_Type
    get_weight(std::size_t device, edge_type const &edge) const
    {
      auto const      &map = weigth_map[device];
      auto             it  = map.find(edge);
      Edge_Weight_Type res;

      if (it == map.cend())
        {
          if (Parent_Type::check_edge(edge))
            {
              throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) +
                                       " was not associated with any weight of device " + std::to_string(device));
            }
          else
            {
              throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
            }
        }

      if constexpr (Parallel_Edges)
        {
          for (; it != map.cend() && it->first == edge; ++it)
            res.insert(it->second);
        }
      else
        {
          if (it != map.cend())
            res = it->second;
        }

      return res;
    }


    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weight The weight
    void
    set_weight(std::size_t device, edge_type const &edge, weight_type const &weight)
    {
      if (!Parent_Type::check_edge(edge))
        {
          throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
        }

      if constexpr (Parallel_Edges)
        {
          weigth_map[device].emplace(edge, weight);
        }
      else
        {
          weigth_map[device][edge] = weight;
        }
    }


    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weights The weights
    void
    set_weight(std::size_t device, edge_type const &edge, Edge_Weight_Type const &weights)
      requires Parallel_Edges
    {
      if (Parent_Type::check_edge(edge))
        {
          for (auto const &weight : weights)
            weigth_map[device].emplace(edge, weight);
        }
      else
        {
          throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
        }
    }

    /// Gets the number of devices
    /// \return Number of devices
    [[nodiscard]] std::size_t
    get_num_devices() const
    {
      return weigth_map.size();
    }

    /// Simple helper function that will print the graph
    /// \return The graph description
    [[nodiscard]] virtual std::string
    print_graph() const
    {
      std::stringstream builder;
      builder << "In Out Weight_Map_Id Weight" << std::endl;

      std::set<std::pair<node_id_type, node_id_type>> recorded_edges;
      for (auto const &node : Parent_Type::nodes)
        {
          for (auto const &out : Parent_Type::get_output_nodes(node.get_id()))
            {
              recorded_edges.emplace_hint(recorded_edges.end(), std::make_pair(node.get_id(), out));
              std::string base_tmp = std::to_string(node.get_id()) + " " + std::to_string(out) + " ";
              for (std::size_t i = 0; i < weigth_map.size(); ++i)
                {
                  auto const &map = weigth_map[i];

                  if constexpr (Parallel_Edges)
                    {
                      std::string new_base = base_tmp + std::to_string(i) + " ";
                      auto        it       = map.find(std::make_pair(node.get_id(), out));
                      if (it != map.cend())
                        {
                          for (auto const &w : it->second)
                            {
                              builder << new_base << Utilities::custom_to_string(w) << std::endl;
                            }
                        }
                      else
                        {
                          builder << new_base << "$" << std::endl;
                        }
                    }
                  else
                    {
                      builder << base_tmp << std::to_string(i) << " ";

                      auto it = map.find(std::make_pair(node.get_id(), out));
                      if (it != map.cend())
                        {
                          builder << Utilities::custom_to_string(it->second);
                        }
                      else
                        {
                          builder << "$";
                        }

                      builder << std::endl;
                    }
                }
            }
        }

      return builder.str();
    }

    ~MWGraph() override = default;
  };

} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_MWGRAPH_H
