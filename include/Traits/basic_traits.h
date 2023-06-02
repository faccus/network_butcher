//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_BASIC_TRAITS_H
#define NETWORK_BUTCHER_BASIC_TRAITS_H

#include "starting_traits.h"
#include "type_info.h"

#include <map>
#include <memory>


#if PARALLEL_TBB
#include <execution>
#elif PARALLEL_OPENMP
#include <omp.h>
#endif

namespace network_butcher
{
  using type_info_pointer = std::shared_ptr<network_butcher::types::Type_info const>;

  template <class T = type_info_pointer>
  using io_collection_type = std::map<std::string, T>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_BASIC_TRAITS_H
