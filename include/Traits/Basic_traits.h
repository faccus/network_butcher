//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_BASIC_TRAITS_H
#define NETWORK_BUTCHER_BASIC_TRAITS_H

#include "Starting_traits.h"
#include "Type_info.h"

#include <map>
#include <memory>


#if PARALLEL
#  include <execution>
#  define SEQ std::execution::seq
#  define PAR_UNSEQ std::execution::par_unseq
#  define PAR std::execution::par
#else
#  define SEQ
#  define PAR_UNSEQ
#  define PAR
#endif

namespace network_butcher
{
  using type_info_pointer = std::shared_ptr<network_butcher::types::Type_info const>;

  template <class T = type_info_pointer>
  using io_collection_type = std::map<std::string, T>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_BASIC_TRAITS_H
