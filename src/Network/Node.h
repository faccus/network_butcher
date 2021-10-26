//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_NODE_H
#define NETWORK_BUTCHER_NODE_H

#include <utility>

#include "../Helpers/Traits/Basic_traits.h"


/// A node of a graph
class Node
{
private:
  /// Current node id
  node_id_type id;

  /// Collection of the ids of inputs of the node
  io_id_collection_type input;
  /// Collection of the ids of outputs of the node
  io_id_collection_type output;
  /// Collection of the ids of parameters of the node
  io_id_collection_type parameters;

public:

  /// Basic constructor for a node
  /// \param starting_id Initial node id
  /// \param initial_input Initial set of inputs
  /// \param initial_output Initial set of outputs
  Node(node_id_type   starting_id,
       io_id_collection_type initial_input,
       io_id_collection_type initial_output,
       io_id_collection_type initial_parameters)
    : id(starting_id)
    , input(std::move(initial_input))
    , output(std::move(initial_output))
    , parameters(std::move(initial_parameters))
  {}


  /// Read-only getter for input
  /// \return Const reference to input
  inline const io_id_collection_type &
  get_input() const
  {
    return input;
  }


  /// Read-only getter for output
  /// \return Const reference to output
  inline const io_id_collection_type &
  get_output() const
  {
    return output;
  }


  /// Read-only getter for the parameters
  /// \return Const reference to the parameters
  inline const io_id_collection_type &
  get_parameters() const
  {
    return parameters;
  }

  inline node_id_type
  get_id() const
  {
    return id;
  }
};

#endif // NETWORK_BUTCHER_NODE_H
