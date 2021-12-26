//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_BASIC_TRAITS_H
#define NETWORK_BUTCHER_BASIC_TRAITS_H

#include <map>
#include <set>


using node_id_type          = std::size_t;
using io_id_type            = int;
using io_id_collection_type = std::map<std::string, io_id_type>;
using operation_id_type     = std::string;


#endif // NETWORK_BUTCHER_BASIC_TRAITS_H
