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



template <class T>
class Graph
{
private:
public:
  std::vector<T> nodes;

  Graph() = default;
  Graph(const std::vector<T> &v)
    : nodes(v)
  {}

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

  std::vector<size_t>
  compute_nodes_memory_usage_output() const
  {
    std::vector<size_t> memory_usages;
    memory_usages.resize(nodes.size());


    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](const T &in) { return in.compute_memory_usage_output(); });

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


using Type_info_pointer = std::shared_ptr<Type_info>;
using Node_type = Node<Type_info_pointer>;
using Layer_type        = Graph<Node_type>;

using Map_IO = std::unordered_map<std::string, Type_info_pointer>;
using Input_graph_type = Node_type;

template<>
class Graph<Input_graph_type>
{
private:

  Map_IO inputs;
  Map_IO outputs;
  Map_IO value_infos;

  std::unordered_map<std::string, int> appearances;

  Map_IO
  onnx_parameters_reader(
    const google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> &params,
    const std::unordered_set<std::string>                          &ignore_set)
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
                appearances.insert(std::make_pair(param.name(), -1));
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

public:
  std::vector<Input_graph_type> nodes;

  Graph() = default;
  Graph(const std::vector<Input_graph_type> &v)
    : nodes(v)
  {};

  Graph(Graph<Input_graph_type> &&) = default;

  Graph(const onnx::ModelProto & model, bool ignore_parameters = false) {
    const auto & in_graph = model.graph();

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

    const auto &in_graph_nodes = in_graph.node();

    nodes.reserve(in_graph_nodes.size());

    auto process_nodes =
      [&](const google::protobuf::RepeatedPtrField<std::basic_string<char>>
                                         &inp,
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
                appearances[in]++;
              }
          }
      };

    for (int i = 0; i < in_graph_nodes.size(); ++i)
      {
        const auto &node = in_graph_nodes[i];

        std::vector<Type_info_pointer> input;
        std::vector<Type_info_pointer> output;

        process_nodes(node.input(), input);
        process_nodes(node.output(), output);

        if (input.size() == 0)
          {
            std::cout << "Error" << std::endl;
          }
        if(output.size() == 0) {
            std::cout << "Error" << std::endl;
          }

        nodes.emplace_back(i, input, output);
      }

    std::cout << appearances["relu0"] << std::endl;
  }

  Graph(const std::string &path)
    : Graph(utilities::parse_onnx_file(path))
  {}

  std::vector<size_t>
  compute_nodes_memory_usage() const
  {
    std::vector<size_t> memory_usages;
    memory_usages.resize(nodes.size());


    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](const Input_graph_type &in) { return in.compute_memory_usage(); });

    return memory_usages;
  }

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

  std::vector<size_t>
  compute_nodes_memory_usage_output() const
  {
    std::vector<size_t> memory_usages;
    memory_usages.resize(nodes.size());


    std::transform(nodes.cbegin(),
                   nodes.cend(),
                   memory_usages.begin(),
                   [](const T &in) { return in.compute_memory_usage_output(); });

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


#endif // NETWORK_BUTCHER_GRAPH_H
