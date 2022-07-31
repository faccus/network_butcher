//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_BASIC_TRAITS_H
#define NETWORK_BUTCHER_BASIC_TRAITS_H

#include <map>
#include <memory>
#include <set>

#include "../Types/Type_info.h"


using type_info_pointer = std::shared_ptr<network_butcher_types::Type_info>;

template <class T = type_info_pointer>
using io_collection_type = std::map<std::string, T>;

using node_id_type          = std::size_t;
using operation_id_type     = std::string;


#endif // NETWORK_BUTCHER_BASIC_TRAITS_H
