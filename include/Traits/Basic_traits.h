//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_BASIC_TRAITS_H
#define NETWORK_BUTCHER_BASIC_TRAITS_H

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>

#include "Type_info.h"

namespace network_butcher
{
  using type_info_pointer = std::shared_ptr<network_butcher::types::Type_info>;

  template <class T = type_info_pointer>
  using io_collection_type = std::map<std::string, T>;

  using node_id_type            = std::size_t;
  using node_id_collection_type = std::set<node_id_type>;
  using edge_type               = std::pair<node_id_type, node_id_type>;


  using operation_id_type       = std::string;
  using weight_type             = double;
  using weights_collection_type = std::map<edge_type, weight_type>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_BASIC_TRAITS_H
