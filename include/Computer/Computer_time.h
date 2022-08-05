//
// Created by faccus on 07/11/21.
//

#ifndef NETWORK_BUTCHER_COMPUTER_TIME_H
#define NETWORK_BUTCHER_COMPUTER_TIME_H

#include "../APSC/Factory.h"
#include "../Traits/Graph_traits.h"

#include "Hardware_specifications.h"

namespace network_butcher_computer
{
  class Computer_time
  {
    using computer_operations = std::function<num_operations_type(node_type const &, bool)>;
    using factory             = GenericFactory::FunctionFactory<operation_id_type, computer_operations>;

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
    compute_operation_time(node_type const &node, Hardware_specifications const &hw);

    [[nodiscard]] inline time_type
    compute_operation_time(network_butcher_types::Graph<graph_input_type> const &graph,
                           node_id_type const                                   &id,
                           Hardware_specifications const                        &hw)
    {
      return compute_operation_time(graph.get_nodes()[id], hw);
    };
  };
} // namespace network_butcher_computer

#endif // NETWORK_BUTCHER_COMPUTER_TIME_H
