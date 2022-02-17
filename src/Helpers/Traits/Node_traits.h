//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_NODE_TRAITS_H
#define NETWORK_BUTCHER_NODE_TRAITS_H

#include <memory>

#include "../../Network/Content.h"
#include "../../Network/Node.h"
#include "../Types/Type_info.h"

using node_type               = Node<Content>;
using node_id_collection_type = std::set<node_id_type>;

using edge_type               = std::pair<node_id_type, node_id_type>;
using weight_type             = double;
using collection_weights_type = std::unordered_map<edge_type, weight_type>;

#endif // NETWORK_BUTCHER_NODE_TRAITS_H
