//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_LAYER_H
#define NETWORK_BUTCHER_LAYER_H

#include <vector>
#include <algorithm>
#include <numeric>

using size_t = std::size_t;

template <class T>
class Layer
{
private:
  int layer_id;
  std::vector<T> nodes;


public:
  Layer() = default;
  Layer(int in_layer_id)
    : layer_id(in_layer_id)
  {}

  void add_node(const T & elem) {
    nodes.push_back(elem);
  }

  const std::vector<T> & get_elem() const {
    return nodes;
  }

  std::vector<T> & get_elem() {
    return nodes;
  }

  std::vector<size_t>
  compute_nodes_memory_usage() const
  {
    std::vector<size_t> memory_usages;
    memory_usages.resize(nodes.size());


    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](const T &in) { return in.compute_memory_usage(); });

    return memory_usages;
  }

  size_t
  compute_memory_usage() const {
    size_t result = 0;
    const std::vector<size_t> memory_usages = compute_nodes_memory_usage();
    std::reduce(memory_usages.cbegin(), memory_usages.cend(), result);

    return result;
  }
};


#endif // NETWORK_BUTCHER_LAYER_H
