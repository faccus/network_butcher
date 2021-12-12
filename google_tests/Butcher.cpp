//
// Created by faccus on 11/09/21.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/Butcher.h"
#include "../src/Helpers/APCS/chrono.h"
#include "TestClass.h"

/*
void
Analyze(const onnx::ValueInfoProto &);
void
Analyze(const onnx::NodeProto &);
void
PrintInputOutput(const onnx::ModelProto &);
*/


TEST(ButcherTest, compute_two_slice_brute_force_test)
{
  using Input = graph_input_type;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  Butcher<Input>    butcher(model_path);
  auto              res = butcher.compute_two_slice_brute_force();
}

TEST(ButcherTest, compute_two_slice_memory_brute_force_test)
{
  using Input = graph_input_type;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";

  Graph<Input>          graph(model_path, true);
  const Computer_memory computer{};

  size_t half_size = computer.compute_memory_usage_input(graph) / 2;

  Butcher<Input> butcher(std::move(graph));

  auto tot = butcher.compute_two_slice_memory_brute_force(half_size);

  std::cout << std::endl;
}


TEST(ButcherTest, compute_k_shortest_paths_eppstein_linear)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::size_t num_devices = 3;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.push_back(node_type(io_id_collection_type{}, {0}));
  nodes.push_back(node_type(io_id_collection_type{0}, {1}));
  nodes.push_back(node_type(io_id_collection_type{1}, {2}));
  nodes.push_back(node_type(io_id_collection_type{1}, {3}));
  nodes.push_back(node_type(io_id_collection_type{3}, {4}));
  nodes.push_back(node_type(io_id_collection_type{2, 4}, {5}));
  nodes.push_back(node_type(io_id_collection_type{5}, {6}));
  nodes.push_back(node_type(io_id_collection_type{6}, {7}));

  for (io_id_type i = 0; i < nodes.size(); ++i)
    map[i] = i;

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));

  std::vector<type_collection_weights> weight_maps;
  weight_maps.reserve(num_devices);

  weight_maps.emplace_back();
  auto &initial_map = weight_maps.back();

  initial_map[{0, 1}] = 1000.;
  initial_map[{1, 2}] = 1000.;
  initial_map[{1, 3}] = 500.;
  initial_map[{3, 4}] = 500.;
  initial_map[{2, 5}] = 1000.;
  initial_map[{4, 5}] = 1000.;
  initial_map[{5, 6}] = 1000.;
  initial_map[{6, 7}] = 0.;

  for (std::size_t k = 1; k < num_devices; ++k)
    {
      weight_maps.emplace_back(weight_maps.front());
      auto &tmp_map = weight_maps.back();
      for (auto &edge : tmp_map)
        edge.second /= std::pow(2, k);
    }

  auto const &graph     = butcher.getGraph();
  auto const  num_nodes = graph.nodes.size();

  std::vector<std::function<type_weight(edge_type const &)>> maps;

  for (std::size_t i = 0; i < num_devices; ++i)
    {
      auto &weight_map = weight_maps[i];

      maps.emplace_back([&weight_map](edge_type const &edge) {
        return weight_map.find(edge)->second;
      });
    }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    transmission_fun =
      [&](node_id_type const &input, std::size_t first, std::size_t second) {
        auto in_device_id  = first;
        auto out_device_id = second;

        if (in_device_id > out_device_id)
          std::swap(in_device_id, out_device_id);

        if (out_device_id - in_device_id == 2)
          return 1000.;
        else if (out_device_id - in_device_id == 1)
          {
            if (out_device_id == 2)
              return 700.;
            else
              return 300.;
          }
        else
          return 0.;
      };


  auto const tmp_res = butcher.compute_k_shortest_paths_eppstein_linear(
    maps, transmission_fun, num_devices, 1000);

  auto const &res = tmp_res.second;

  for (auto i = 0; i < res.size(); ++i)
    for (auto j = 0; j < res.size(); ++j)
      if (i != j && !(res[i] < res[j] || res[j] < res[i]))
        ASSERT_TRUE(!(res[i] < res[j] || res[j] < res[i]));

  ASSERT_EQ(res.size(), 81);
}

TEST(ButcherTest, compute_k_shortest_paths_lazy_eppstein_linear)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::size_t num_devices = 3;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.push_back(node_type(io_id_collection_type{}, {0}));
  nodes.push_back(node_type(io_id_collection_type{0}, {1}));
  nodes.push_back(node_type(io_id_collection_type{1}, {2}));
  nodes.push_back(node_type(io_id_collection_type{1}, {3}));
  nodes.push_back(node_type(io_id_collection_type{3}, {4}));
  nodes.push_back(node_type(io_id_collection_type{2, 4}, {5}));
  nodes.push_back(node_type(io_id_collection_type{5}, {6}));
  nodes.push_back(node_type(io_id_collection_type{6}, {7}));

  for (io_id_type i = 0; i < nodes.size(); ++i)
    map[i] = i;

  Graph<Input>   graph_cons(nodes, map);
  Butcher<Input> butcher(std::move(graph_cons));

  std::vector<type_collection_weights> weight_maps;
  weight_maps.reserve(num_devices);

  weight_maps.emplace_back();
  auto &initial_map = weight_maps.back();

  initial_map[{0, 1}] = 1000.;
  initial_map[{1, 2}] = 1000.;
  initial_map[{1, 3}] = 500.;
  initial_map[{3, 4}] = 500.;
  initial_map[{2, 5}] = 1000.;
  initial_map[{4, 5}] = 1000.;
  initial_map[{5, 6}] = 1000.;
  initial_map[{6, 7}] = 0.;

  for (std::size_t k = 1; k < num_devices; ++k)
    {
      weight_maps.emplace_back(weight_maps.front());
      auto &tmp_map = weight_maps.back();
      for (auto &edge : tmp_map)
        edge.second /= std::pow(2, k);
    }

  auto const &graph     = butcher.getGraph();
  auto const  num_nodes = graph.nodes.size();

  std::vector<std::function<type_weight(edge_type const &)>> maps;

  for (std::size_t i = 0; i < num_devices; ++i)
    {
      auto &weight_map = weight_maps[i];

      maps.emplace_back([&weight_map](edge_type const &edge) {
        return weight_map.find(edge)->second;
      });
    }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    transmission_fun =
      [&](node_id_type const &input, std::size_t first, std::size_t second) {
        auto in_device_id  = first;
        auto out_device_id = second;

        if (in_device_id > out_device_id)
          std::swap(in_device_id, out_device_id);

        if (out_device_id - in_device_id == 2)
          return 1000.;
        else if (out_device_id - in_device_id == 1)
          {
            if (out_device_id == 2)
              return 700.;
            else
              return 300.;
          }
        else
          return 0.;
      };


  auto const tmp_res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
    maps, transmission_fun, num_devices, 1000);

  auto const &res = tmp_res.second;

  for (auto i = 0; i < res.size(); ++i)
    for (auto j = 0; j < res.size(); ++j)
      if (i != j && !(res[i] < res[j] || res[j] < res[i]))
        ASSERT_TRUE(!(res[i] < res[j] || res[j] < res[i]));

  ASSERT_EQ(res.size(), 81);
}





TEST(ButcherTest, compute_k_shortest_paths_test_network)
{
  std::string path = "resnet18-v2-7-inferred.onnx";

  Butcher<graph_input_type> butcher(path);

  auto const &graph     = butcher.getGraph();
  auto const &nodes     = graph.nodes;
  auto const  num_nodes = nodes.size();

  std::size_t num_devices = 3;

  std::random_device         rd;
  std::default_random_engine random_engine{rd()};

  std::uniform_real_distribution node_weights_generator{5000., 10000.};

  std::vector<type_collection_weights> weight_maps;
  weight_maps.reserve(num_devices);

  weight_maps.emplace_back();
  auto &initial_weight_map = weight_maps.back();

  for (node_id_type i = 0; i < num_nodes - 1; ++i)
    initial_weight_map[{i, i + 1}] = node_weights_generator(random_engine);

  for (std::size_t k = 1; k < num_devices; ++k)
    {
      weight_maps.emplace_back(weight_maps.front());
      auto &tmp_map = weight_maps.back();
      for (auto &edge : tmp_map)
        edge.second /= std::pow(2, k);
    }

  std::vector<std::function<type_weight(edge_type const &)>> maps;

  for (std::size_t i = 0; i < num_devices; ++i)
    {
      auto &weight_map = weight_maps[i];

      maps.emplace_back([&weight_map](edge_type const &edge) {
        return weight_map.find(edge)->second;
      });
    }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
    transmission_fun =
      [&](node_id_type const &input, std::size_t first, std::size_t second) {
        auto in_device_id  = first;
        auto out_device_id = second;

        if (in_device_id > out_device_id)
          std::swap(in_device_id, out_device_id);

        if (out_device_id - in_device_id == 2)
          return 1000.;
        else if (out_device_id - in_device_id == 1)
          {
            if (out_device_id == 2)
              return 700.;
            else
              return 300.;
          }
        else
          return 0.;
      };


  Chrono crono;
  crono.start();
  auto const tmp_res = butcher.compute_k_shortest_paths_lazy_eppstein_linear(
    maps, transmission_fun, num_devices, num_nodes / 10);
  crono.stop();

  std::cout << "Lazy Eppstein: " << crono.wallTime() / 1000 << " milliseconds"
            << std::endl;

  crono.start();
  auto res = butcher.reconstruct_model(tmp_res.first, tmp_res.second.front());
  crono.stop();

  std::cout << "Model reconstruction: " << crono.wallTime() / 1000
            << " milliseconds" << std::endl;
}


/*
TEST(ButcherTest, butcher_test_zone)
{
  std::string const path       = "resnet18-v2-7-inferred.onnx";
  std::string const res_path_1 = "resnet18-v2-7-inferred_1.onnx";
  std::string const res_path_2 = "resnet18-v2-7-inferred_2.onnx";

  onnx::ModelProto model = utilities::parse_onnx_file(path);

  PrintInputOutput(model);

  onnx::ModelProto first_half;
  onnx::ModelProto second_half;

  first_half.set_doc_string(model.doc_string());
  first_half.set_domain(model.domain());
  first_half.set_producer_name(model.producer_name());
  first_half.set_producer_version(model.producer_version());

  second_half.set_doc_string(model.doc_string());
  second_half.set_domain(model.domain());
  second_half.set_producer_name(model.producer_name());
  second_half.set_producer_version(model.producer_version());

  auto first_graph  = new onnx::GraphProto;
  auto second_graph = new onnx::GraphProto;
  auto graph        = model.graph();

  first_graph->clear_node();
  second_graph->clear_node();

  first_graph->clear_input();
  second_graph->clear_input();

  first_graph->clear_output();
  second_graph->clear_output();

  first_graph->clear_initializer();
  second_graph->clear_initializer();

  first_graph->clear_sparse_initializer();
  second_graph->clear_sparse_initializer();

  first_graph->clear_value_info();
  second_graph->clear_value_info();


  auto                                   nodes             = graph.node();
  std::size_t                            nodes_first_graph = 12;
  std::set<onnx::ValueInfoProto const *> first_input;

  std::function<void(onnx::GraphProto *, onnx::NodeProto *)> add_to_graph =
    [&graph](onnx::GraphProto *sup_graph, onnx::NodeProto *node) {
      for (std::size_t i = 0; i < node->input_size(); ++i)
        {
          auto it = std::find_if(graph.input().begin(),
                                 graph.input().end(),
                                 [&node, &i](onnx::ValueInfoProto const &ref) {
                                   return node->input(i) == ref.name();
                                 });

          if (it != graph.input().end())
            {
              auto const tmp = sup_graph->add_input();
              *tmp           = *it;
            }
          else
            {
              it = std::find_if(graph.value_info().begin(),
                                graph.value_info().end(),
                                [&node, &i](onnx::ValueInfoProto const &ref) {
                                  return node->input(i) == ref.name();
                                });

              if (it != graph.value_info().end())
                {
                  auto const tmp = sup_graph->add_value_info();
                  *tmp           = *it;
                }
            }

          auto init = std::find_if(graph.initializer().begin(),
                                   graph.initializer().end(),
                                   [&node, &i](onnx::TensorProto const &ref) {
                                     return node->input(i) == ref.name();
                                   });
          if (init != graph.initializer().end())
            {
              auto const tmp = sup_graph->add_initializer();
              *tmp           = *init;
            }
        }
    };

  for (std::size_t i = 0; i < nodes_first_graph; ++i)
    {
      auto tmp = first_graph->add_node();
      *tmp     = nodes.Get(i);

      add_to_graph(first_graph, tmp);
    }

  for (std::size_t i = first_graph->node_size(); i < nodes.size(); ++i)
    {
      auto tmp = second_graph->add_node();
      *tmp     = nodes.Get(i);

      add_to_graph(second_graph, tmp);
    }

  {
    auto tmp_in                  = second_graph->add_input();
    auto in_node                 = *(--first_graph->node().end());
    auto communication_node_name = *in_node.output().begin();

    auto tmp_out = first_graph->add_output();

    auto res = std::find_if(model.graph().input().begin(),
                            model.graph().input().end(),
                            [communication_node_name](auto const &ref) {
                              return ref.name() == communication_node_name;
                            });

    if (res == model.graph().input().end())
      res = std::find_if(model.graph().output().begin(),
                         model.graph().output().end(),
                         [communication_node_name](auto const &ref) {
                           return ref.name() == communication_node_name;
                         });
    if (res == model.graph().output().end())
      res = std::find_if(model.graph().value_info().begin(),
                         model.graph().value_info().end(),
                         [communication_node_name](auto const &ref) {
                           return ref.name() == communication_node_name;
                         });

    tmp_in->set_name(communication_node_name);
    tmp_out->set_name(communication_node_name);

    if (res != model.graph().input().end())
      {
        tmp_in->set_allocated_type(new onnx::TypeProto(res->type()));
        tmp_out->set_allocated_type(new onnx::TypeProto(res->type()));
      }
  }


  first_half.set_allocated_graph(first_graph);
  second_half.set_allocated_graph(second_graph);

  std::cout << std::endl;
}
void
Analyze(const onnx::ValueInfoProto &info)
{
  auto &first_input_type = info.type();
  std::cout << info.name() << " ";

  if (first_input_type.IsInitialized())
    {
      if (first_input_type.has_map_type())
        {
          auto &type = first_input_type.map_type();
          std::cout << std::endl;
        }

      if (first_input_type.has_optional_type())
        {
          auto &type = first_input_type.optional_type();
          std::cout << std::endl;
        }

      if (first_input_type.has_sequence_type())
        {
          auto &type = first_input_type.sequence_type();
          std::cout << std::endl;
        }

      if (first_input_type.has_tensor_type())
        {
          const auto &type = first_input_type.tensor_type();

          const auto elem_type = type.elem_type();
        }

      if (first_input_type.has_sparse_tensor_type())
        {
          auto &type = first_input_type.sparse_tensor_type();
        }
    }

  std::cout << std::endl;
}
void
Analyze(const onnx::NodeProto &node)
{
  std::cout << "Node " << node.name() << std::endl;

  auto &input  = node.input();
  auto &output = node.output();

  std::cout << "Input: " << std::endl;
  for (const auto &s : input)
    std::cout << s << std::endl;

  std::cout << "Output: " << std::endl;
  for (const auto &s : output)
    std::cout << s << std::endl;


  std::cout << std::endl;
}

void
PrintInputOutput(const onnx::ModelProto &mod)
{
  auto &graph      = mod.graph();
  auto &inputs     = graph.input();
  auto &outputs    = graph.output();
  auto &value_info = graph.value_info();

  std::cout << "General Inputs: " << std::endl << std::endl;
  for (auto &in : inputs)
    Analyze(in);

  std::cout << "General Outputs: " << std::endl << std::endl;
  for (auto &out : outputs)
    Analyze(out);

  std::cout << "Value_info: " << std::endl << std::endl;
  for (auto &vi : value_info)
    Analyze(vi);

  std::cout << "Initializers: " << std::endl << std::endl;
  for (auto &init : graph.initializer())
    std::cout << init.name() << std::endl;
}
*/