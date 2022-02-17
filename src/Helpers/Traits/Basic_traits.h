//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_BASIC_TRAITS_H
#define NETWORK_BUTCHER_BASIC_TRAITS_H

#include <map>
#include <memory>
#include <set>

#include "../Types/Type_info.h"


using type_info_pointer = std::shared_ptr<Type_info>;
using graph_input_type  = type_info_pointer;

using node_id_type          = std::size_t;
using io_id_type            = ulong;
using io_id_collection_type = std::map<std::string, Type_info>;
using operation_id_type     = std::string;


#endif // NETWORK_BUTCHER_BASIC_TRAITS_H
