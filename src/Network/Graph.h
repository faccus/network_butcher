//
// Created by faccus on 24/08/21.
//

#ifndef NETWORK_BUTCHER_GRAPH_H
#define NETWORK_BUTCHER_GRAPH_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>


#include "../Helpers/Types/Type_info.h"
#include "../Helpers/Types/Dense_tensor.h"

#include "../Onnx_model/onnx.pb.h"
#include "Layer.h"
#include "Node.h"

/// Just another graph class...
/// \tparam T Type of the node in the graph
template <class T>
class Graph
{
private:
public:
  /// Vector of all the nodes
  std::vector<T> nodes;

  Graph() = default;

  /// Construct the graph from the nodes
  Graph(const std::vector<T> &v)
    : nodes(v)
  {}

  /// Compute the memory usage of all the nodes in the graph (method required by the node: <size_t>(): compute_memory_usage() )
  /// \return Vector containing the memory usage of every node
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

  /// Compute the memory usage of all inputs of all the nodes in the graph (method required by the node: <size_t>(): compute_memory_usage_input() )
  /// \return Vector containing the total memory usage of the inputs of every node
  std::vector<size_t>
  compute_nodes_memory_usage_input() const
  {
    std::vector<size_t> memory_usages;
    memory_usages.resize(nodes.size());


    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](const T &in) { return in.compute_memory_usage_input(); });

    return memory_usages;
  }

  /// Compute the memory usage of all outputs of all the nodes in the graph (method required by the node: <size_t>(): compute_memory_usage_input() )
  /// \return Vector containing the total memory usage of the outputs of every node
  std::vector<size_t>
  compute_nodes_memory_usage_output() const
  {
    std::vector<size_t> memory_usages;
    memory_usages.resize(nodes.size());


    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](const T &in) {
                     return in.compute_memory_usage_output();
                   });

    return memory_usages;
  }

  /// Compute the total memory usage of the nodes of the graph
  /// \return Total memory usage of all the nodes of the graph
  size_t
  compute_memory_usage() const
  {
    size_t                    result        = 0;
    const std::vector<size_t> memory_usages = compute_nodes_memory_usage();
    result = std::reduce(memory_usages.cbegin(), memory_usages.cend());

    return result;
  }

  /// Compute the total memory usage of all the inputs of all nodes of the graph
  /// \return Total memory usage of all the inputs of all nodes of the graph
  size_t
  compute_memory_usage_input() const
  {
    size_t                    result = 0;
    const std::vector<size_t> memory_usages =
      compute_nodes_memory_usage_input();
    result = std::reduce(memory_usages.cbegin(), memory_usages.cend());

    return result;
  }

  /// Compute the total memory usage of all the outputs of all nodes of the graph
  /// \return Total memory usage of all the outputs of all nodes of the graph
  size_t
  compute_memory_usage_output() const
  {
    size_t                    result = 0;
    const std::vector<size_t> memory_usages =
      compute_nodes_memory_usage_output();
    result = std::reduce(memory_usages.cbegin(), memory_usages.cend());

    return result;
  }
};


using Type_info_pointer = std::shared_ptr<Type_info>;
using Node_type = Node<Type_info>;

using Map_IO = std::unordered_map<std::string, Type_info_pointer>;
using Input_graph_type = Node_type;

template<>
class Graph<Input_graph_type>
{
private:

  /// Maps the name of an input of the model with the respective type
  Map_IO inputs;
  /// Maps the name of an output of the model with the respective type
  Map_IO outputs;
  /// Maps the name of an value infos of the model with the respective type
  Map_IO value_infos;

  /// Have the dependencies of every node already computed?
  bool dependencies_computed = false;

  /// From a ValueInfoProto (type of an input/output/value_info of the graph) collection of elements, it will construct a map that associated the name of the ValueInfoProto to the respective type
  /// \param params Collection of ValueInfoProto
  /// \param ignore_set The names of the parameters to ignore
  /// \return The map that associated the name of the ValueInfoProto to the respective type
  Map_IO
  onnx_parameters_reader(
    const google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> &params,
    const std::unordered_set<std::string>                          &ignore_set) const
  {
    Map_IO out;
    for (const auto &param : params)
      {
        if (param.IsInitialized() && !ignore_set.contains(param.name()))
          {
            const auto type = param.type();
            if (type.has_map_type())
              {
                std::cout << std::endl;
              }

            if (type.has_optional_type())
              {
                std::cout << std::endl;
              }

            if (type.has_sequence_type())
              {
                std::cout << std::endl;
              }

            if (type.has_tensor_type())
              {
                auto &obj = type.tensor_type();

                out[param.name()] = std::make_shared<Dense_tensor>(param);
              }

            if (type.has_sparse_tensor_type())
              {
                std::cout << std::endl;
              }
          }
        else
          {
            std::cout << std::endl;
          }
      }

    return out;
  }


  /// It will produce the set of all the nodes that have a Type_info in types and that appear in the map
  /// \param types The collection of Type_info
  /// \param appearances The map that associated to the name of a Type_info a collection of nodes
  /// \return The set of all the nodes that have a Type_info in types and that appear in the map
  std::set<int>
  find_nodes(
    const std::vector<Type_info_pointer>                    &types,
    const std::unordered_map<std::string, std::vector<int>> &appearances) const
  {
    std::set<int> res;
    for (auto &out : types)
      {
        auto p = appearances.find(out->get_name());
        if (p != appearances.end())
          {
            auto &tmp_output_nodes = (*p).second;
            res.insert(tmp_output_nodes.begin(), tmp_output_nodes.end());
          }
      }
    return res;
  }

  /// An helper function used to compute the dependencies of every node of the graph
  void
  helper_compute_dependencies() {

    {
      std::vector<std::pair<std::set<int>, std::set<int>>> tmp;
      dependencies = tmp;
    }

    for(int i = 0; i < nodes.size(); ++i) {

        auto & node = nodes[i];

        // To get the input for a node, I have to look at the output of the others.
        auto in = find_nodes(node.get_input(), appearances_output);
        auto out = find_nodes(node.get_output(), appearances_input);

        dependencies.push_back( {in, out} );
      }
  }

public:

  /// Collection of nodes
  std::vector<Input_graph_type> nodes;

  /// Map that associates the names of TypeInfos with the nodes that have as an input a Typeinfo
  std::unordered_map<std::string, std::vector<int>> appearances_input;
  /// Map that associates the names of TypeInfos with the nodes that have as an output a Typeinfo
  std::unordered_map<std::string, std::vector<int>> appearances_output;

  std::vector< std::pair<std::set<int>, std::set<int> > > dependencies;


  Graph() = default;
  Graph(const std::vector<Input_graph_type> &v)
    : nodes(v)
  {};


  Graph(Graph<Input_graph_type> &&) = default;

  /// Construct a graph from a model
  /// \param model Protobuf model
  /// \param ignore_parameters Ignore the inputs/outputs already initialized
  /// \param dependencies Compute the nodes dependencies
  Graph(const onnx::ModelProto &model,
        bool                    ignore_parameters = false,
        bool                    dependencies      = true)
  {
    const auto &in_graph = model.graph();

    {
      std::unordered_set<std::string> ignore_set;
      if (ignore_parameters)
        {
          for (auto &p : in_graph.initializer())
            if (p.IsInitialized())
              ignore_set.insert(p.name());
        }

      inputs      = onnx_parameters_reader(in_graph.input(), ignore_set);
      outputs     = onnx_parameters_reader(in_graph.output(), ignore_set);
      value_infos = onnx_parameters_reader(in_graph.value_info(), ignore_set);
    }

    // Vector of nodes of the graph
    const auto &in_graph_nodes = in_graph.node();

    nodes.reserve(in_graph_nodes.size());

    // Base on the names of the input/output, it will produce the associated
    // type
    auto process_nodes =
      [&](
        const google::protobuf::RepeatedPtrField<std::basic_string<char>> &inp,
        std::vector<Type_info_pointer> &ing) {
        for (auto &in : inp)
          {
            Map_IO ::const_iterator p     = inputs.find(in);
            bool                    valid = p != inputs.cend();

            if (!valid)
              {
                p     = outputs.find(in);
                valid = p != outputs.cend();
              }
            if (!valid)
              {
                p     = value_infos.find(in);
                valid = p != value_infos.cend();
              }

            if (valid)
              {
                ing.push_back(p->second);
              }
          }
      };

    // Based on the name of the input/output, it will link it with the
    // respective nodes
    auto add_appearances =
      [&](const std::vector<Type_info_pointer>              &in,
          int                                               &index,
          std::unordered_map<std::string, std::vector<int>> &appearances) {
        for (auto &i : in)
          appearances[i->get_name()].push_back(index);
      };

    int current_level = 0;
    int node_index    = -1;

    for (int i = 0; i < in_graph_nodes.size(); ++i)
      {
        const auto &node = in_graph_nodes[i];

        std::vector<Type_info_pointer> input;
        std::vector<Type_info_pointer> output;

        process_nodes(node.input(), input);
        process_nodes(node.output(), output);


        if (! (input.size() == 0 && ignore_parameters) )
          {
            auto current_index = ++node_index;

            add_appearances(input, current_index, appearances_input);
            add_appearances(output, current_index, appearances_output);

            nodes.emplace_back(current_index, input, output);
          }
      }

    if (dependencies)
      compute_dependencies();
  }


  /// Construct a graph from a Protobuf Model stored at the specified path
  /// \param path The absolute/relative path of the .onnx model
  /// \param ignore_parameters Ignore the inputs/outputs already initialized
  /// \param dependencies Compute the nodes dependencies
  Graph(const std::string &path,
        bool               ignore_parameters = false,
        bool               dependencies      = true)
    : Graph(utilities::parse_onnx_file(path), ignore_parameters, dependencies)
  {}

  /// Compute the memory usage of all the nodes in the graph (method required by the node: <size_t>(): compute_memory_usage() )
  /// \return Vector containing the memory usage of every node
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

  /// Compute the memory usage of all inputs of all the nodes in the graph (method required by the node: <size_t>(): compute_memory_usage_input() )
  /// \return Vector containing the total memory usage of the inputs of every node
  std::vector<size_t>
  compute_nodes_memory_usage_input() const
  {
    std::vector<size_t> memory_usages;
    memory_usages.resize(nodes.size());


    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](const T &in) { return in.compute_memory_usage_input(); });

    return memory_usages;
  }

  /// Compute the memory usage of all outputs of all the nodes in the graph (method required by the node: <size_t>(): compute_memory_usage_input() )
  /// \return Vector containing the total memory usage of the outputs of every node
  std::vector<size_t>
  compute_nodes_memory_usage_output() const
  {
    std::vector<size_t> memory_usages;
    memory_usages.resize(nodes.size());


    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](const T &in) {
                     return in.compute_memory_usage_output();
                   });

    return memory_usages;
  }

  /// Compute the total memory usage of the nodes of the graph
  /// \return Total memory usage of all the nodes of the graph
  size_t
  compute_memory_usage() const
  {
    size_t                    result        = 0;
    const std::vector<size_t> memory_usages = compute_nodes_memory_usage();
    result = std::reduce(memory_usages.cbegin(), memory_usages.cend());

    return result;
  }

  /// Compute the total memory usage of all the inputs of all nodes of the graph
  /// \return Total memory usage of all the inputs of all nodes of the graph
  size_t
  compute_memory_usage_input() const
  {
    size_t                    result = 0;
    const std::vector<size_t> memory_usages =
      compute_nodes_memory_usage_input();
    result = std::reduce(memory_usages.cbegin(), memory_usages.cend());

    return result;
  }

  /// Compute the total memory usage of all the outputs of all nodes of the graph
  /// \return Total memory usage of all the outputs of all nodes of the graph
  size_t
  compute_memory_usage_output() const
  {
    size_t                    result = 0;
    const std::vector<size_t> memory_usages =
      compute_nodes_memory_usage_output();
    result = std::reduce(memory_usages.cbegin(), memory_usages.cend());

    return result;
  }

  /// Compute node dependencies, if they were not already computed
  /// \param forced Force the computation of the dependencies, even if they were already computed.
  /// \return Return true if the dependencies were computed, false otherwise
  bool
  compute_dependencies(bool forced = false) {
    if(!forced && dependencies_computed)
      return false;

    helper_compute_dependencies();
    dependencies_computed = true;
    return true;
  }
};


#endif // NETWORK_BUTCHER_GRAPH_H
