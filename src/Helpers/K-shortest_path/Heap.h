//
// Created by faccus on 28/10/21.
//

#ifndef NETWORK_BUTCHER_HEAP_H
#define NETWORK_BUTCHER_HEAP_H

#include "../Traits/Basic_traits.h"
#include <utility>
#include <vector>

template <class T>
class Heap
{
public:
  using children_type = std::set<T>;

  children_type children;

  Heap() = default;

  Heap(children_type children)
    : children{std::move(children)}
  {}

  void
  merge(children_type const &to_merge)
  {
    children.reserve(children.size() + to_merge.children.size());
    children.merge(to_merge.children);
  }
};


#endif // NETWORK_BUTCHER_HEAP_H
