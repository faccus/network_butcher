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

public:
  Computer_time();

  void
  setup() const;

  [[nodiscard]] time_type
  compute_operation_time(Graph<graph_input_type> const &graph,
                         Node const                    &node,
                         const Hardware_specifications &hw);

  [[nodiscard]] inline time_type
  compute_operation_time(Graph<graph_input_type> const &graph,
                         node_id_type                   id,
                         const Hardware_specifications &hw);
};


#endif // NETWORK_BUTCHER_COMPUTER_TIME_H
