//
// Created by faccus on 25/04/23.
//

#ifndef NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H
#define NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H

#include "Block_Graph_Builder.h"

namespace network_butcher
{
  template <typename GraphType>
  class Constrained_Block_Graph_Builder : public Block_Graph_Builder<GraphType>
  {
  protected:
    using Parent = Block_Graph_Builder<GraphType>;

    std::vector<std::unique_ptr<constraints::Extra_Constraint>> constraints;
    bool                                                        backward_connections_allowed;
    parameters::Block_Graph_Generation_Mode                     block_graph_mode;

    node_id_type starting_device_id;
    node_id_type ending_device_id;

  public:
    explicit Constrained_Block_Graph_Builder(GraphType const &original_graph)
      : Block_Graph_Builder<GraphType>{original_graph} {};

    explicit Constrained_Block_Graph_Builder(GraphType const &original_graph, parameters::Parameters const &params)
      : Block_Graph_Builder<GraphType>{original_graph}
      , backward_connections_allowed{params.backward_connections_allowed}
      , block_graph_mode{params.block_graph_mode}
      , starting_device_id{params.starting_device_id}
      , ending_device_id{params.ending_device_id}
    {
      if (params.memory_constraint != parameters::Memory_Constraint_Type::None)
        {
          constraints.emplace_back(std::make_unique<constraints::Memory_Constraint>(params.memory_constraint));
        }
    };

    void
    add_constraint(std::unique_ptr<constraints::Extra_Constraint> &&constraint);

    void
    construct_block_graph();

    void
    apply_constraints();

    void
    apply_weights();


    ~Constrained_Block_Graph_Builder() = default;
  };


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::add_constraint(
    std::unique_ptr<constraints::Extra_Constraint> &&constraint)
  {
    constraints.emplace_back(std::move(constraint));
  };


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::construct_block_graph()
  {
    using Parent::original_graph;

    auto const linearize_graph = [](GraphType::Dependencies_Type const                             &old_dependencies,
                                    GraphType::Node_Collection_Type const                          &old_nodes,
                                    network_butcher::parameters::Block_Graph_Generation_Mode const &mode) {
      // Counter is used to establish if the current node has more output
      // connections than the inputs one.
      int counter = old_dependencies.front().second.size() - old_dependencies.front().first.size() - 1;

      std::list<block_graph_type::Node_Type> starting_nodes;
      std::map<node_id_type, node_id_type>   old_to_new; // Old node -> New node


      starting_nodes.emplace_back(block_graph_type::Node_Internal_Type{0, nullptr});
      starting_nodes.back().content.second = std::make_shared<node_id_collection_type>(node_id_collection_type{0});

      // Cycle through all the nodes of the graph
      for (auto it = ++old_nodes.begin(); it != old_nodes.end(); ++it)
        {
          // Node of the old graph
          auto const &node          = *it;
          auto const &dep           = old_dependencies[node.get_id()];
          int const   local_counter = dep.second.size() - dep.first.size();

          // Add new node
          if (local_counter <= 0 && counter == 0)
            {
              starting_nodes.emplace_back(block_graph_type::Node_Internal_Type{0, nullptr});
              starting_nodes.back().content.second =
                std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});
            }
          // Add new node and add master node for next steps
          else if (local_counter > 0 && counter == 0)
            {
              starting_nodes.emplace_back(block_graph_type::Node_Internal_Type{0, nullptr});
              starting_nodes.back().content.second =
                std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});

              starting_nodes.emplace_back(block_graph_type::Node_Internal_Type{0, nullptr});
              starting_nodes.back().content.second = std::make_shared<node_id_collection_type>();

              counter += local_counter;
            }
          // Add node link to the "big" node
          else if ((local_counter == 0 && dep.second.size() == 1 || local_counter > 0 && dep.first.size() <= 1) &&
                   counter > 0)
            {
              starting_nodes.back().content.second->insert(starting_nodes.back().content.second->end(), node.get_id());

              counter += local_counter;
            }
          else if (counter > 0 && ((local_counter >= 0 && dep.first.size() > 1) || (local_counter < 0)))
            {
              counter -= (dep.first.size() - 1);

              // End of the master node
              if (counter == 0)
                {
                  starting_nodes.emplace_back(block_graph_type::Node_Internal_Type{0, nullptr});
                  starting_nodes.back().content.second =
                    std::make_shared<node_id_collection_type>(node_id_collection_type{node.get_id()});

                  // Do we have to add another master node?
                  if (local_counter >= 0)
                    {
                      starting_nodes.emplace_back(block_graph_type::Node_Internal_Type{0, nullptr});
                      starting_nodes.back().content.second = std::make_shared<node_id_collection_type>();
                    }
                }
              else
                {
                  starting_nodes.back().content.second->insert(starting_nodes.back().content.second->end(),
                                                               node.get_id());
                }

              counter += (dep.second.size() - 1);
            }
          else
            {
              throw std::runtime_error("Unknown node found during block_graph construction");
            }
        }

      if (starting_nodes.size() > 1)
        {
          if (mode == network_butcher::parameters::Block_Graph_Generation_Mode::input)
            {
              for (auto it = ++starting_nodes.begin(), it_follower = starting_nodes.begin(); it != starting_nodes.end();
                   ++it, ++it_follower)
                {
                  if (it->content.second->size() > 1)
                    {
                      auto &content_big_node = it->content.second;

                      it_follower->content.second->insert(std::make_move_iterator(content_big_node->begin()),
                                                          std::make_move_iterator(content_big_node->end()));

                      starting_nodes.erase(it);
                      it = ++it_follower;

                      if (it == starting_nodes.end())
                        break;
                    }
                }
            }
          else if (mode == network_butcher::parameters::Block_Graph_Generation_Mode::output)
            {
              for (auto it          = ++starting_nodes.begin(),
                        it_follower = starting_nodes.begin(),
                        it_mem      = starting_nodes.begin();
                   it != starting_nodes.end();
                   ++it, ++it_follower)
                {
                  if (it->content.second->size() > 1)
                    {
                      it_mem = it_follower;
                    }
                  else if (it_follower->content.second->size() > 1)
                    {
                      auto &content_small_node = it->content.second;

                      it_follower->content.second->insert(std::make_move_iterator(content_small_node->begin()),
                                                          std::make_move_iterator(content_small_node->end()));

                      starting_nodes.erase(it);
                      it          = it_follower;
                      it_follower = it_mem;
                    }
                }
            }
        }

      return std::tuple{starting_nodes, old_to_new};
    };

    auto const add_extra_nodes_per_device = [](std::list<block_graph_type::Node_Type> &starting_nodes,
                                               std::size_t                             num_devices) {
      for (auto it_follower = ++starting_nodes.cbegin(), it = ++(++starting_nodes.cbegin());
           it != starting_nodes.cend();
           it_follower = it, ++it)
        {
          for (std::size_t i = 1; i < num_devices; ++i)
            {
              starting_nodes.emplace(it, block_graph_type::Node_Internal_Type{i, it_follower->content.second});
            }
        }
    };

    auto const process_dependencies =
      [](std::size_t dep_size, std::size_t supp_size, std::size_t num_devices, bool backward_allowed) {
        block_graph_type::Dependencies_Type new_dependencies;
        new_dependencies.reserve(dep_size);

        // The first node is fully connected with the first layer
        {
          new_dependencies.emplace_back();

          auto &out = new_dependencies.back().second;
          for (std::size_t k = 0; k < num_devices; ++k)
            out.insert(out.end(), k + 1);
        }

        // Inputs: first node, Outputs: following layer nodes
        {
          new_dependencies.emplace_back(std::make_pair<node_id_collection_type, node_id_collection_type>({0}, {}));
          auto &out = new_dependencies.back().second;
          for (std::size_t k = 0; k < num_devices; ++k)
            out.insert(out.end(), 1 + num_devices + k);

          for (std::size_t k = 1; k < num_devices; ++k)
            {
              auto dep_cpy = new_dependencies.back();

              if (!backward_allowed)
                dep_cpy.second.erase(num_devices + k);

              new_dependencies.push_back(std::move(dep_cpy));
            }
        }

        // Inputs: previous layer nodes, Outputs: following layer nodes
        {
          for (std::size_t i = 2; i < supp_size; ++i)
            {
              auto const id = new_dependencies.size();
              new_dependencies.emplace_back(std::make_pair<node_id_collection_type, node_id_collection_type>({}, {}));

              auto &in  = new_dependencies.back().first;
              auto &out = new_dependencies.back().second;

              if (!backward_allowed)
                in.insert(in.end(), id - num_devices);


              for (std::size_t k = 0; k < num_devices; ++k)
                {
                  if (backward_allowed)
                    in.insert(in.end(), id - num_devices + k);

                  out.insert(out.end(), id + num_devices + k);
                }

              for (std::size_t k = 1; k < num_devices; ++k)
                {
                  auto tmp_dep = new_dependencies.back();

                  if (!backward_allowed)
                    {
                      tmp_dep.first.insert(id - num_devices + k);
                      tmp_dep.second.erase(id + num_devices + k - 1);
                    }

                  new_dependencies.emplace_back(std::move(tmp_dep));
                }
            }
        }

        // Inputs: previous layer nodes, Outputs: last node
        {
          auto const id = new_dependencies.size();

          new_dependencies.emplace_back(
            std::make_pair<node_id_collection_type, node_id_collection_type>({}, {id + num_devices}));

          auto &in = new_dependencies.back().first;
          in.insert(in.end(), id - num_devices);

          if (backward_allowed)
            for (std::size_t k = 1; k < num_devices; ++k)
              in.insert(in.end(), id - num_devices + k);

          for (std::size_t k = 1; k < num_devices; ++k)
            {
              auto dep_cpy = new_dependencies.back();

              if (!backward_allowed)
                dep_cpy.first.insert(id - num_devices + k);

              new_dependencies.emplace_back(std::move(dep_cpy));
            }
        }

        // The last layer is fully connected with the last node
        {
          new_dependencies.emplace_back(std::make_pair<node_id_collection_type, node_id_collection_type>({}, {}));

          auto &in = new_dependencies.back().first;
          for (std::size_t k = 0; k < num_devices; ++k)
            in.insert(in.end(), new_dependencies.size() - 1 - num_devices + k);
        }

        return std::move(new_dependencies);
      };

    auto [starting_nodes, tmp_old_to_new] =
      linearize_graph(original_graph.get_neighbors(), original_graph.get_nodes(), block_graph_mode);

    this->old_to_new = std::move(tmp_old_to_new);

    auto const supp_size = starting_nodes.size() - 2;
    if (starting_nodes.size() > 2)
      add_extra_nodes_per_device(starting_nodes, original_graph.get_num_devices());

    block_graph_type::Node_Collection_Type new_nodes;
    new_nodes.reserve(starting_nodes.size());

    new_nodes.insert(new_nodes.end(),
                     std::make_move_iterator(starting_nodes.begin()),
                     std::make_move_iterator(starting_nodes.end()));

    new_nodes.front().content.first = starting_device_id;
    new_nodes.back().content.first  = ending_device_id;

    this->new_graph =
      block_graph_type(new_nodes,
                       process_dependencies(new_nodes.size(), supp_size, original_graph.get_num_devices()),
                       backward_connections_allowed);
  };


  template <typename GraphType>
  void
  Constrained_Block_Graph_Builder<GraphType>::apply_constraints()
  {
    for (auto const &constraint : constraints)
      {
        constraint->apply_constraint(this->new_graph);
      }
  };
} // namespace network_butcher

#endif // NETWORK_BUTCHER_CONSTRAINED_BLOCK_GRAPH_BUILDER_H
