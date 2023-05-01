//
// Created by faccus on 24/04/23.
//

#ifndef NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H
#define NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H

#include "Computer_memory.h"
#include "Graph_traits.h"
#include "Parameters.h"

namespace network_butcher::constraints
{
  class Extra_Constraint
  {
  public:
    Extra_Constraint() = default;

    virtual void
    apply_constraint(block_graph_type &graph) const = 0;

    [[nodiscard]] virtual std::unique_ptr<Extra_Constraint>
    copy() const;

    virtual ~Extra_Constraint() = default;
  };


  template <typename GraphType>
  class Memory_Constraint : public Extra_Constraint
  {
  private:
    parameters::Parameters const &params;
    GraphType const              &graph;

    [[nodiscard]] std::tuple<memory_type, memory_type>
    estimate_maximum_memory_usage(const std::vector<network_butcher::parameters::Device> &devices,
                                  network_butcher::parameters::Memory_Constraint_Type     constraint_type,
                                  const std::set<node_id_type>                           &ids,
                                  const std::vector<memory_type>                         &input_memory,
                                  const std::vector<memory_type>                         &output_memory,
                                  const std::vector<memory_type>                         &params_memory) const;

  public:
    explicit Memory_Constraint(parameters::Parameters const &params, GraphType const &graph)
      : params{params}
      , graph{graph} {};

    void
    apply_constraint(block_graph_type &graph) const override;

    [[nodiscard]] std::unique_ptr<Extra_Constraint>
    copy() const override;


    ~Memory_Constraint() override = default;
  };

  template <typename GraphType>
  std::unique_ptr<Extra_Constraint>
  Memory_Constraint<GraphType>::copy() const
  {
    return std::make_unique<Memory_Constraint<GraphType>>(*this);
  }

  template <typename GraphType>
  void
  Memory_Constraint<GraphType>::apply_constraint(block_graph_type &new_graph) const
  {
    auto const &constraint_type = params.memory_constraint_type;

    if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::None)
      return;

    auto const            &devices = params.devices;
    std::set<node_id_type> to_remove;

    auto const input_memory  = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_input(graph);
    auto const output_memory = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_output(graph);
    auto const params_memory = network_butcher::computer::Computer_memory::compute_nodes_memory_usage_parameters(graph);

    std::vector<bool> available(devices.size(), true);
    memory_type       memory_graph = 0;


    auto const dependencies_clear = [&](node_id_type const &node_id) {
      auto &dep = new_graph.get_neighbors_ref()[node_id];

      for (auto const &in : dep.first)
        new_graph.get_neighbors_ref()[in].second.erase(node_id);
      for (auto const &out : dep.second)
        new_graph.get_neighbors_ref()[out].first.erase(node_id);

      dep.first.clear();
      dep.second.clear();
    };

    auto const response_fun_max = [&](std::size_t basic_node_id, memory_type const &memory_node) {
      if (memory_graph < memory_node)
        {
          memory_graph = std::max(memory_graph, memory_node);

          for (std::size_t k = 0; k < devices.size(); ++k)
            {
              if (available[k] && memory_graph < devices[k].maximum_memory)
                {
                  continue;
                }
              else
                {
                  available[k] = false;
                  dependencies_clear(basic_node_id + k);
                  to_remove.insert(basic_node_id + k);
                }
            }
        }
    };

    auto const response_fun_preload_parameters =
      [&](std::size_t basic_node_id, memory_type const &param, memory_type const &io) {
        memory_graph += param;

        for (std::size_t k = 0; k < devices.size(); ++k)
          {
            if (available[k] && (memory_graph + io) < devices[k].maximum_memory)
              {
                continue;
              }
            else
              {
                available[k] = false;
                dependencies_clear(basic_node_id + k);
                to_remove.insert(basic_node_id + k);
              }
          }
      };

    for (std::size_t i = 1; i < new_graph.size() - 1; i += devices.size())
      {
        auto const &new_node_content = *new_graph[i].content.second;
        bool        easy_content     = new_node_content.size() == 1;

        if (easy_content)
          {
            auto const index = *new_node_content.begin();

            if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Max)
              {
                response_fun_max(i, params_memory[index] + input_memory[index] + output_memory[index]);
              }
            else if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
              {
                response_fun_preload_parameters(i, params_memory[index], input_memory[index] + output_memory[index]);
              }
          }
        else
          {
            auto const [io_mem, param_mem] = estimate_maximum_memory_usage(
              devices, constraint_type, new_node_content, input_memory, output_memory, params_memory);

            if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Max)
              {
                response_fun_max(i, param_mem + io_mem);
              }
            else if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
              {
                response_fun_preload_parameters(i, param_mem, io_mem);
              }
          }
      }

    new_graph.remove_nodes(to_remove);
  }

  template <typename GraphType>
  std::tuple<memory_type, memory_type>
  Memory_Constraint<GraphType>::estimate_maximum_memory_usage(
    const std::vector<network_butcher::parameters::Device> &devices,
    network_butcher::parameters::Memory_Constraint_Type     constraint_type,
    const std::set<node_id_type>                           &ids,
    const std::vector<memory_type>                         &input_memory,
    const std::vector<memory_type>                         &output_memory,
    const std::vector<memory_type>                         &params_memory) const
  {
    memory_type result_memory = 0, fixed_memory = 0;

    if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
      {
        fixed_memory = std::reduce(std::next(params_memory.begin(), *ids.cbegin()),
                                   std::next(params_memory.begin(), *ids.crbegin()));
      }
    auto const &dependencies = graph.get_neighbors();
    std::size_t qty          = 1;


    if (dependencies[*ids.begin()].first.size() == 1)
      {
        auto const &father = *dependencies[*ids.begin()].first.begin();
        qty                = std::max(qty, dependencies[father].second.size());
      }

    for (auto const &id : ids)
      {
        auto const &node = graph[id];

        auto const &node_dependencies = dependencies[id];
        auto const &parents           = node_dependencies.first;
        auto const &children          = node_dependencies.second;

        memory_type        in_memory  = input_memory[id];
        memory_type const &out_memory = output_memory[id];

        if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Preload_Parameters)
          {
            result_memory = std::max(result_memory, in_memory + out_memory);
          }
        else if (constraint_type == network_butcher::parameters::Memory_Constraint_Type::Max)
          {
            result_memory = std::max(result_memory, in_memory + out_memory + params_memory[id]);
          }

        if (children.size() > parents.size())
          qty = qty + children.size() - parents.size();
      }

    return {result_memory * qty, fixed_memory};
  }


  template <typename GraphType>
  std::function<std::vector<std::unique_ptr<constraints::Extra_Constraint>>()>
  generate_constraint_function(parameters::Parameters const &params, GraphType const &graph)
  {
    return [&params, &graph]() {
      std::vector<std::unique_ptr<Extra_Constraint>> res;

      if (params.memory_constraint)
        {
          res.push_back(std::make_unique<Memory_Constraint<GraphType>>(params, graph));
        }

      return res;
    };
  };
} // namespace network_butcher::constraints

#endif // NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H
