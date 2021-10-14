//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_NODE_TRAITS_H
#define NETWORK_BUTCHER_NODE_TRAITS_H

#import "../../Network/Node.h"

using node_type = Node<Type_info>;
using type_info_pointer = std::shared_ptr<Type_info>;
using graph_input_type  = node_type;

#endif // NETWORK_BUTCHER_NODE_TRAITS_H
