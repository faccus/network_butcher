//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_NODE_TRAITS_H
#define NETWORK_BUTCHER_NODE_TRAITS_H

#include <memory>

#include "content.h"
#include "node.h"
#include "type_info.h"

namespace network_butcher
{
  using graph_input_type = network_butcher::types::CNode<network_butcher::types::Content<type_info_pointer>>;
  using block_graph_input_type =
    network_butcher::types::CNode<std::pair<std::size_t, std::shared_ptr<node_id_collection_type>>>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_NODE_TRAITS_H
