//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_NODE_TRAITS_H
#define NETWORK_BUTCHER_NODE_TRAITS_H

#include <memory>

#include "Content.h"
#include "Node.h"
#include "Type_info.h"

namespace network_butcher
{
  using graph_input_type = network_butcher::types::Content<type_info_pointer>;
  using node_type        = network_butcher::types::Node<graph_input_type>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_NODE_TRAITS_H
