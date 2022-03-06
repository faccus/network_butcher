//
// Created by faccus on 28/10/21.
//

#ifndef NETWORK_BUTCHER_HEAP_H
#define NETWORK_BUTCHER_HEAP_H

#include "../Traits/Basic_traits.h"
#include "../Traits/Node_traits.h"
#include <utility>
#include <vector>

template <class T>
class Heap
{
public:
  using children_type = std::set<T>;

  children_type children;
  node_id_type  id;


  Heap() = default;

  explicit Heap(children_type children)
    : children{std::move(children)}
  {
  }

  explicit Heap(std::vector<T> const children)
    : children{std::move(children)}
  {}
};


#endif // NETWORK_BUTCHER_HEAP_H
