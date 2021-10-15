//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_NODE_TRAITS_H
#define NETWORK_BUTCHER_NODE_TRAITS_H

#include "../../Network/Node.h"

using node_type = Node;
using type_info_pointer = std::shared_ptr<Type_info>;
using graph_input_type  = type_info_pointer;
using node_id_collection_type = std::set<node_id_type>;

#endif // NETWORK_BUTCHER_NODE_TRAITS_H
