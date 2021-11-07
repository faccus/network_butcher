//
// Created by faccus on 07/11/21.
//

#ifndef NETWORK_BUTCHER_COMPUTER_TIME_H
#define NETWORK_BUTCHER_COMPUTER_TIME_H

#include "../../Network/Graph.h"
#include "../Traits/Hardware_traits.h"

class Computer_time
{
public:
  [[nodiscard]] time_type
  compute_time_usage(Graph<graph_input_type> const &graph, Node const &node);

  [[nodiscard]] inline time_type
  compute_time_usage(Graph<graph_input_type> const &graph, node_id_type id);
};


#endif // NETWORK_BUTCHER_COMPUTER_TIME_H
