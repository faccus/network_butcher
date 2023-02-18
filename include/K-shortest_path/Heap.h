//
// Created by faccus on 28/10/21.
//

#ifndef NETWORK_BUTCHER_HEAP_H
#define NETWORK_BUTCHER_HEAP_H

#include "Basic_traits.h"
#include "Node_traits.h"

#include <utility>
#include <vector>

namespace network_butcher_kfinder
{
  template <class T>
  class Heap
  {
  public:
    using container_type = std::set<T>;

    node_id_type   id;
    container_type children;

    Heap() = default;

    explicit Heap(node_id_type id, container_type children)
      : id(id)
      , children{std::move(children)}
    {}
  };
} // namespace network_butcher_kfinder


#endif // NETWORK_BUTCHER_HEAP_H
