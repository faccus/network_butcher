#ifndef NETWORK_BUTCHER_MWGRAPH_H
#define NETWORK_BUTCHER_MWGRAPH_H

#include "graph.h"


namespace network_butcher::types
{
  /// A custom graph class. It contains a single graph and multiple weight maps. Technically, it can be viewed
  /// as a collection of graphs with the same structure, but different weight maps.
  /// \tparam Parallel_Edges If true, the graph will allow parallel edges
  /// \tparam Template_Node_Type The type of the node
  /// \tparam t_weight_type The type of the weight
  template <bool Parallel_Edges, typename Template_Node_Type = Node, typename t_weight_type = Time_Type>
  class MWGraph : public Graph<Template_Node_Type>
  {
  protected:
    using Parent_Type = Graph<Template_Node_Type>;


  public:
    using Dependencies_Type    = Parent_Type::Neighbours_Type;
    using Node_Type            = Parent_Type::Node_Type;
    using Node_Collection_Type = Parent_Type::Node_Collection_Type;

    using Weight_Type = t_weight_type;

  private:
    using single_edge_weight_container = std::vector<std::map<Node_Id_Type, Weight_Type>>;
    using multi_edge_weight_container  = std::vector<std::multimap<Node_Id_Type, Weight_Type>>;

  public:
    using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<Weight_Type>, Weight_Type>;
    using Weight_Collection_Type =
      std::conditional_t<Parallel_Edges, multi_edge_weight_container, single_edge_weight_container>;

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
      for (auto &map : weigth_map)
        {
          map.resize(Parent_Type::size());
        }
    }


    /// Checks if the given edge has a weight on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] auto
    check_weight(std::size_t device, Edge_Type const &edge) const -> bool
    {
      return weigth_map[device][edge.first].contains(edge.second);
    }


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] auto
    get_weight(std::size_t device, Edge_Type const &edge) const
    {
      if (device >= weigth_map.size())
        {
          throw std::runtime_error("MWGraph: the device " + Utilities::custom_to_string(device) + " does not exist");
        }

      auto const &map = weigth_map[device][edge.first];
      auto        it  = map.find(edge.second);

      Edge_Weight_Type res;
      if (it == map.cend())
        {
          if (Parent_Type::check_edge(edge))
            {
              throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) +
                                       " was not associated with any weight of device " +
                                       Utilities::custom_to_string(device));
            }
          else
            {
              throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
            }
        }

      if constexpr (Parallel_Edges)
        {
          for (; it != map.cend() && it->first == edge.second; ++it)
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
    set_weight(std::size_t device, Edge_Type const &edge, t_weight_type const &weight)
    {
      if (!Parent_Type::check_edge(edge))
        {
          throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
        }

      if constexpr (Parallel_Edges)
        {
          weigth_map[device][edge.first].emplace(edge.second, weight);
        }
      else
        {
          weigth_map[device][edge.first][edge.second] = weight;
        }
    }

    /// Sets the weights for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weights The weights
    void
    set_weight(std::size_t device, Edge_Type const &edge, Edge_Weight_Type weights)
      requires Parallel_Edges
    {
      if (Parent_Type::check_edge(edge))
        {
          for (auto const &weight : weights)
            weigth_map[device][edge.first].emplace(edge.second, weight);
        }
      else
        {
          throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
        }
    }


    /// Gets the number of devices
    /// \return Number of devices
    [[nodiscard]] auto
    get_num_devices() const
    {
      return weigth_map.size();
    }

    /// Simple helper function that will print the graph
    /// \return The graph description
    [[nodiscard]] virtual auto
    print_graph() const -> std::string
    {
      std::stringstream builder;
      builder << "In Out Weight_Map_Id Weight" << std::endl;

      std::set<std::pair<Node_Id_Type, Node_Id_Type>> recorded_edges;
      for (auto const &node : Parent_Type::nodes)
        {
          for (auto const &out : Parent_Type::get_output_nodes(node.get_id()))
            {
              recorded_edges.emplace_hint(recorded_edges.end(), std::make_pair(node.get_id(), out));
              std::string base_tmp =
                Utilities::custom_to_string(node.get_id()) + " " + Utilities::custom_to_string(out) + " ";
              for (std::size_t i = 0; i < weigth_map.size(); ++i)
                {
                  auto const &map = weigth_map[i];

                  if constexpr (Parallel_Edges)
                    {
                      std::string new_base = base_tmp + Utilities::custom_to_string(i) + " ";
                      auto        it       = map[node.get_id()].find(out);
                      if (it != map[node.get_id()].cend())
                        {
                          for (; it != map[node.get_id()].cend() && it->first == out; ++it)
                            {
                              builder << new_base << Utilities::custom_to_string(it->second) << std::endl;
                            }
                        }
                      else
                        {
                          builder << new_base << "$" << std::endl;
                        }
                    }
                  else
                    {
                      builder << base_tmp << Utilities::custom_to_string(i) << " ";

                      auto it = map[node.get_id()].find(out);

                      if (it != map[node.get_id()].cend())
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
  /// \tparam Parallel_Edges If true, the graph will allow parallel edges
  /// \tparam T The content type of each CNode
  template <bool Parallel_Edges, typename T>
  class MWGraph<Parallel_Edges, CNode<Content<T>>, Time_Type> : public Graph<CNode<Content<T>>>
  {
  private:
    using Parent_Type = Graph<CNode<Content<T>>>;

  public:
    using Dependencies_Type    = Parent_Type::Neighbours_Type;
    using Node_Type            = Parent_Type::Node_Type;
    using Node_Collection_Type = Parent_Type::Node_Collection_Type;

    using Weight_Type = Time_Type;

  private:
    using single_edge_weight_container = std::vector<std::map<Node_Id_Type, Weight_Type>>;
    using multi_edge_weight_container  = std::vector<std::multimap<Node_Id_Type, Weight_Type>>;

  public:
    using Edge_Weight_Type = std::conditional_t<Parallel_Edges, std::multiset<Weight_Type>, Weight_Type>;
    using Weight_Collection_Type =
      std::conditional_t<Parallel_Edges, multi_edge_weight_container, single_edge_weight_container>;

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
      for (auto &map : weigth_map)
        {
          map.resize(Parent_Type::size());
        }
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
      for (auto &map : weigth_map)
        {
          map.resize(Parent_Type::size());
        }
    }


    /// Checks if the given edge has a weight on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return True if the edge has a weight on the given device, false otherwise
    [[nodiscard]] auto
    check_weight(std::size_t device, Edge_Type const &edge) const -> bool
    {
      return weigth_map[device][edge.first].contains(edge.second);
    }


    /// Get the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \return The weight
    [[nodiscard]] auto
    get_weight(std::size_t device, Edge_Type const &edge) const -> Edge_Weight_Type
    {
      if (device >= weigth_map.size())
        {
          throw std::runtime_error("MWGraph: the device " + Utilities::custom_to_string(device) + " does not exist");
        }

      auto const &map = weigth_map[device][edge.first];
      auto        it  = map.find(edge.second);

      Edge_Weight_Type res;
      if (it == map.cend())
        {
          if (Parent_Type::check_edge(edge))
            {
              throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) +
                                       " was not associated with any weight of device " +
                                       Utilities::custom_to_string(device));
            }
          else
            {
              throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
            }
        }

      if constexpr (Parallel_Edges)
        {
          for (; it != map.cend() && it->first == edge.second; ++it)
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
    set_weight(std::size_t device, Edge_Type const &edge, Time_Type const &weight)
    {
      if (!Parent_Type::check_edge(edge))
        {
          throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
        }

      if constexpr (Parallel_Edges)
        {
          weigth_map[device][edge.first].emplace(edge.second, weight);
        }
      else
        {
          weigth_map[device][edge.first][edge.second] = weight;
        }
    }


    /// Sets the weight for the given edge on the given device
    /// \param device The device id
    /// \param edge The edge
    /// \param weights The weights
    void
    set_weight(std::size_t device, Edge_Type const &edge, Edge_Weight_Type const &weights)
      requires Parallel_Edges
    {
      if (Parent_Type::check_edge(edge))
        {
          for (auto const &weight : weights)
            weigth_map[device][edge.first].emplace(edge.second, weight);
        }
      else
        {
          throw std::runtime_error("MWGraph: the edge " + Utilities::custom_to_string(edge) + " does not exist");
        }
    }


    /// Gets the number of devices
    /// \return Number of devices
    [[nodiscard]] auto
    get_num_devices() const
    {
      return weigth_map.size();
    }


    /// Simple helper function that will print the graph
    /// \return The graph description
    [[nodiscard]] virtual auto
    print_graph() const -> std::string
    {
      std::stringstream builder;
      builder << "In Out Weight_Map_Id Weight" << std::endl;

      std::set<std::pair<Node_Id_Type, Node_Id_Type>> recorded_edges;
      for (auto const &node : Parent_Type::nodes)
        {
          for (auto const &out : Parent_Type::get_output_nodes(node.get_id()))
            {
              recorded_edges.emplace_hint(recorded_edges.end(), std::make_pair(node.get_id(), out));
              std::string base_tmp =
                Utilities::custom_to_string(node.get_id()) + " " + Utilities::custom_to_string(out) + " ";
              for (std::size_t i = 0; i < weigth_map.size(); ++i)
                {
                  auto const &map = weigth_map[i];

                  if constexpr (Parallel_Edges)
                    {
                      std::string new_base = base_tmp + Utilities::custom_to_string(i) + " ";
                      auto        it       = map[node.get_id()].find(out);
                      if (it != map.cend())
                        {
                          for (; it != map[node.get_id()].cend() && it->first == out; ++it)
                            {
                              builder << new_base << Utilities::custom_to_string(it->second) << std::endl;
                            }
                        }
                      else
                        {
                          builder << new_base << "$" << std::endl;
                        }
                    }
                  else
                    {
                      builder << base_tmp << Utilities::custom_to_string(i) << " ";

                      auto it = map[node.get_id()].find(out);
                      if (it != map[node.get_id()].cend())
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
