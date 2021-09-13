//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

using size_t = std::size_t;

// T is a type that has compute_memory_usage as a method (shared)
template <class T>
class Node
{
private:
  using io_type = std::vector<std::shared_ptr<T>>;

  int id;
  io_type input;
  io_type    output;

public:
  Node(int                                    starting_id,
       const io_type &initial_input,
       const io_type &initial_output)
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

  const io_type &
  get_input() const
  {
    return input;
  }

  const io_type &
  get_output() const
  {
    return output;
  }

};

#endif // NETWORK_BUTCHER_NODE_H
