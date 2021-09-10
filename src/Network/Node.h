//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

using size_t = std::size_t;

// T is a pointer (either shared or unique)
template <class T>
class Node
{
private:
  int id;
  std::vector<T>   input;
  std::vector<T>   output;

public:
  Node(int                   starting_id,
       const std::vector<T> &initial_input,
       const std::vector<T> &initial_output)
    : id(starting_id)
    , input(initial_input)
    , output(initial_output)
  {}

  size_t
  compute_memory_usage_input() const
  {
    size_t res = 0;
    for(auto & a : input)
      res += a->compute_memory_usage();
    return res;
  }

  size_t
  compute_memory_usage_output() const
  {
    size_t res = 0;
    for(auto & a : output)
      res += a->compute_memory_usage();
    return res;
  }

  size_t
  compute_memory_usage() const
  {
    return compute_memory_usage_input() + compute_memory_usage_output();
  };
};

#endif // NETWORK_BUTCHER_NODE_H
