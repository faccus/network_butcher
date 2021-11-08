//
// Created by faccus on 07/11/21.
//

#ifndef NETWORK_BUTCHER_COMPUTER_TIME_H
#define NETWORK_BUTCHER_COMPUTER_TIME_H

#include "../../Network/Graph.h"
#include "../APCS/Factory.h"
#include "../Traits/Hardware_traits.h"

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
  void
  setup() const;

  [[nodiscard]] time_type
  compute_time_usage(Graph<graph_input_type> const &graph, Node const &node);

  [[nodiscard]] inline time_type
  compute_time_usage(Graph<graph_input_type> const &graph, node_id_type id);
};


#endif // NETWORK_BUTCHER_COMPUTER_TIME_H
