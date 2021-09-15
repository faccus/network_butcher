//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

using size_t = std::size_t;

// T is a type that has compute_memory_usage as a method (shared)

/// A node of a graph
/// \tparam T
template <class T>
class Node
{
private:
  using io_type = std::vector<std::shared_ptr<T>>;

  /// Current node id
  int id;

  /// Collection of the inputs of the vector
  io_type input;
  /// Collection of the outputs of the vector
  io_type    output;

public:

  /// Basic constructor for a node
  /// \param starting_id Initial node id
  /// \param initial_input Initial set of inputs
  /// \param initial_output Initial set of outputs
  Node(int                                    starting_id,
       const io_type &initial_input,
       const io_type &initial_output)
    : id(starting_id)
    , input(initial_input)
    , output(initial_output)
  {}


  /// Compute the memory size of the inputs of the node
  /// \return Memory size of the inputs of the node
  size_t
  compute_memory_usage_input() const
  {
    size_t res = 0;
    for(auto & a : input)
      res += a->compute_memory_usage();
    return res;
  }


  /// Compute the memory size of the outputs of the node
  /// \return Memory size of the outputs of the node
  size_t
  compute_memory_usage_output() const
  {
    size_t res = 0;
    for(auto & a : output)
      res += a->compute_memory_usage();
    return res;
  }


  /// Compute the total memory size of the node
  /// \return Total memory size of the node
  size_t
  compute_memory_usage() const
  {
    return compute_memory_usage_input() + compute_memory_usage_output();
  };


  /// Read-only getter for input
  /// \return Const reference to input
  const io_type &
  get_input() const
  {
    return input;
  }


  /// Read-only getter for output
  /// \return Const reference to output
  const io_type &
  get_output() const
  {
    return output;
  }

};

#endif // NETWORK_BUTCHER_NODE_H
