//
// Created by faccus on 07/11/21.
//

#include "Computer_time.h"

Computer_time::Computer_time()
{
  setup();
}

void
Computer_time::setup() const
{
  auto &factory = get_factory();

  factory.add("relu",
              [](Graph<graph_input_type> const &graph,
                 Node const                    &node,
                 bool                           forward) {
                std::size_t res = -1;
                auto const  it =
                  graph.nodes_content.find(node.get_output().begin()->second);
                if (it == graph.nodes_content.cend())
                  return res;

                std::size_t const C_out = it->second->get_shape()[1];
                res                     = forward ? 3 * C_out : 4 * C_out;

                return res;
              });

  factory.add("loss",
              [](Graph<graph_input_type> const &graph,
                 Node const                    &node,
                 bool                           forward) {
                std::size_t res = 0;
                auto const  it =
                  graph.nodes_content.find(node.get_output().begin()->second);
                if (it == graph.nodes_content.cend())
                  return res;

                std::size_t const C_out = it->second->get_shape()[1];
                res                     = forward ? 4 * C_out - 1 : C_out + 1;

                return res;
              });

  factory.add("batchnormalization",
              [](Graph<graph_input_type> const &graph,
                 Node const                    &node,
                 bool                           forward) {
                std::size_t res = 0;
                auto const  it =
                  graph.nodes_content.find(node.get_output().begin()->second);
                if (it == graph.nodes_content.cend())
                  return res;

                auto const it2 =
                  graph.nodes_content.find(node.get_input().begin()->second);
                if (it2 == graph.nodes_content.cend())
                  return res;

                std::size_t const C_in  = it2->second->get_shape()[1];
                std::size_t const C_out = it->second->get_shape()[1];
                res = forward ? 5 * C_out + C_in - 2 : 8 * C_out + C_in - 1;

                return res;
              });

  factory.add(
    "conv",
    [](Graph<graph_input_type> const &graph, Node const &node, bool forward) {
      std::size_t res = 0;
      auto const  out_iterator =
        graph.nodes_content.find(node.get_output().begin()->second);
      if (out_iterator == graph.nodes_content.cend())
        return res;

      auto const in_iterator =
        graph.nodes_content.find(node.get_input().begin()->second);
      if (in_iterator == graph.nodes_content.cend())
        return res;

      auto const kernel_iterator = node.get_attributes().find("kernel_shape");
      if (kernel_iterator == node.get_attributes().cend())
        return res;

      std::size_t const H_f_times_W_f =
        kernel_iterator->second[0] * kernel_iterator->second[1];

      std::size_t const C_in  = in_iterator->second->get_shape()[1];
      std::size_t const C_out = out_iterator->second->get_shape()[1];


      res = forward ? H_f_times_W_f * C_in * C_out :
                      (2 * H_f_times_W_f * C_in + 1) * C_out;

      return res;
    });

  factory.add(
    "maxpool",
    [](Graph<graph_input_type> const &graph, Node const &node, bool forward) {
      std::size_t res = 0;
      auto const  out_iterator =
        graph.nodes_content.find(node.get_output().begin()->second);
      if (out_iterator == graph.nodes_content.cend())
        return res;

      auto const kernel_iterator = node.get_attributes().find("kernel_shape");
      if (kernel_iterator == node.get_attributes().cend())
        return res;

      std::size_t const C_out = out_iterator->second->get_shape()[1];

      std::size_t const H_f_times_W_f =
        kernel_iterator->second[0] * kernel_iterator->second[1];

      res = forward ? H_f_times_W_f * C_out : (H_f_times_W_f + 1) * C_out;

      return res;
    });

  factory.add(
    "add",
    [](Graph<graph_input_type> const &graph, Node const &node, bool forward) {
      std::size_t res = 0;
      auto const  out_iterator =
        graph.nodes_content.find(node.get_output().begin()->second);
      if (out_iterator == graph.nodes_content.cend())
        return res;

      std::size_t const prod_out = out_iterator->second->get_shape()[0] *
                                   out_iterator->second->get_shape()[1] *
                                   out_iterator->second->get_shape()[2] *
                                   out_iterator->second->get_shape()[3];
      res = prod_out;

      return res;
    });
}

time_type
Computer_time::compute_operation_time(const Graph<graph_input_type> &graph,
                                      const Node                    &node,
                                      const Hardware_specifications &hw)
{
  time_type res = .0;

  auto const &operation_id = graph.nodes_operations[node.get_id()];
  auto       &factory      = get_factory();
  auto const &time_coeffs  = hw.get_regression_coefficients(operation_id);

  if (factory.registered(operation_id) && time_coeffs.first >= 0 &&
      time_coeffs.second >= 0)
    {
      auto       computer   = factory.get(operation_id);
      auto const operations = computer(graph, node, true);
      res = operations * time_coeffs.second + time_coeffs.first;
    }

  return res;
}