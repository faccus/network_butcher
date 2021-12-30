//
// Created by faccus on 07/11/21.
//

#ifndef NETWORK_BUTCHER_COMPUTER_TIME_H
#define NETWORK_BUTCHER_COMPUTER_TIME_H

#include "../../Network/Graph.h"
#include "../APCS/Factory.h"
#include "Hardware_specifications.h"

class Computer_time
{
  using computer_operations = std::function<
    num_operations_type(Graph<graph_input_type> const &, Node const &, bool)>;
  using factory =
    GenericFactory::FunctionFactory<operation_id_type, computer_operations>;

  inline static factory &
  get_factory()
  {
    return factory::Instance();
  }

  void
  setup() const;

public:
  Computer_time();


  [[nodiscard]] time_type
  compute_operation_time(Graph<graph_input_type> const &graph,
                         Node const                    &node,
                         Hardware_specifications const &hw);

  [[nodiscard]] inline time_type
  compute_operation_time(Graph<graph_input_type> const &graph,
                         node_id_type const            &id,
                         Hardware_specifications const &hw)
  {
    return compute_operation_time(graph, graph.nodes[id], hw);
  };
};


#endif // NETWORK_BUTCHER_COMPUTER_TIME_H
