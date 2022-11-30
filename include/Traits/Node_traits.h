//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_NODE_TRAITS_H
#define NETWORK_BUTCHER_NODE_TRAITS_H

#include <memory>

#include "../Network/Node.h"
#include "../Types/Content.h"
#include "../Types/Type_info.h"

using graph_input_type        = network_butcher_types::Content<type_info_pointer>;
using node_type               = network_butcher_types::Node<graph_input_type>;
using node_id_collection_type = std::set<node_id_type>;

using edge_type = std::pair<node_id_type, node_id_type>;

using weight_type             = double;
using weights_collection_type = std::map<edge_type, weight_type>; //, boost::hash<edge_type>>;

#endif // NETWORK_BUTCHER_NODE_TRAITS_H
