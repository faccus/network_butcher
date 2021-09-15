//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_BUTCHER_H
#define NETWORK_BUTCHER_BUTCHER_H

#include "Network/Graph.h"
#include "Network/Node.h"
#include "Helpers/Types/Type_info.h"
#include "Helpers/Utilities.h"


using Type_info_pointer = std::shared_ptr<Type_info>;
using Node_type = Node<Type_info>;

using Layer_type        = Graph<Node_type>;
using In_graph_type = Node_type;

using Slice_type = std::set<int>;

/// Butcher butchers a given graph into slices
class Butcher
{
private:
  using Network = Graph<In_graph_type>;

  Network graph;

  // WIP
  int
  recursive_brute_forcer(std::set<Slice_type> &,
                         const std::vector<Slice_type> &,
                         Slice_type &,
                         int,
                         int mode = -1,
                         int end  = 0);

  /// Compute the minimal connected graphs (with dependencies) containing every node
  /// \return returns the smallest connected sub-graph (with dependencies) that connects the first node with the n-th node
  std::vector<Slice_type>
  compute_basic_routes() const;

  /// Given a vector of slices, verify which ones applied to tester return true. Note that, at the end, all the ok slices will be moved to the return vector, while the non-compatible ones will be deleted
  /// \param slices The vector of input slices
  /// \param tester The tester function
  /// \return The slices that satisfy the tester function
  static std::vector<Slice_type>
  partition_checker(std::vector<Slice_type> & slices,
                    const std::function<bool(const Slice_type &)>& tester) ;


public:
  Butcher() = default;
  /// Move constructor
  explicit Butcher(Network &&g)
    : graph(std::move(g)){};

  /// Read from a file the model, construct the associated graph and prepare the
  /// butcher
  /// \param p Full path to the .onnx file model
  /// \param ignore_parameters Allows to choose if graph should ignore already initialized inputs/outputs (parameters)
  /// \param dep Make the graph compute the dependencies map
  Butcher(const std::string &p, bool ignore_parameters = true, bool dep = true)
    : graph(p, ignore_parameters, dep){};

  /// It will compute every possible 2-slice partition of the network and it will select the partition whose total memory usage is less than the specified value
  /// \param memory_first_slice Total memory usage allowed to the first slice
  /// \return a collection of all the admissible partitions (and the nodes contained in the first partition)
  std::vector<Slice_type>
  compute_two_slice_memory_brute_force(size_t memory_first_slice) const;

  /// It will compute the basic_routes partitions and it will select those whose total memory usage is less than the specified value
  /// \param memory_first_slice Total memory usage allowed to the first slice
  /// \return a collection of all the admissible partitions (and the nodes contained in the first partition)
  std::vector<Slice_type>
  compute_partial_two_slice_memory_brute_force(size_t memory_first_slice) const;

  /// It will try to compute every 2-slice partition of a graph
  /// \return A vector containing every possibile 2-slice partition of the graph (taking into account dependencies)
  std::vector<Slice_type>
  compute_two_slice_brute_force() const;

  // WIP
  std::vector<Slice_type>
  partial_compute_two_slice_brute_force() const;
};


#endif // NETWORK_BUTCHER_BUTCHER_H
