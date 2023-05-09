//
// Created by faccus on 01/05/23.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "TestClass.h"

#include "Constrained_Block_Graph_Builder.h"
#include "Graph_traits.h"

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;

  using type_weight             = double;
  using type_collection_weights = std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  using basic_type   = int;
  using Input        = TestMemoryUsage<int>;
  using Content_type = Content<Input>;
  using Node_type    = Node<Content_type>;
  using GraphType    = MWGraph<Content_type>;

  GraphType basic_graph(std::size_t);
  GraphType basic_graph2(std::size_t);

  parameters::Parameters
  eppstein_parameters(std::size_t k, std::size_t num_devices);

  parameters::Parameters
  lazy_eppstein_parameters(std::size_t k, std::size_t num_devices);

  template <class Graph>
  void
  complete_weights(Graph &graph)
  {
    auto const  num_nodes    = graph.get_nodes().size();
    auto const &dependencies = graph.get_neighbors();

    for (node_id_type tail = 0; tail < num_nodes; ++tail)
      for (auto const &head : dependencies[tail].second)
        {
          for (std::size_t k = 0; k < graph.get_num_devices(); ++k)
            {
              if (graph.get_weight(k, {tail, head}) == -1.)
                graph.set_weight(k, {tail, head}, 0.);
            }
        }
  };


  TEST(BlockGraphBuilderTest, Devices)
  {
    auto graph  = basic_graph(2);
    auto params = eppstein_parameters(5, 2);

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    for (std::size_t i = 1; i < 9; ++i)
      {
        auto const &node = block_graph[i];

        ASSERT_EQ(node.content.first, (i + 1) % 2);
      }
  }


  TEST(BlockGraphBuilderTest, ConstructionClassic)
  {
    auto graph  = basic_graph(2);
    auto params = eppstein_parameters(5, 2);

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    // Same device, same content
    for (std::size_t i = 0; i <= 3; ++i)
      {
        ASSERT_EQ(block_graph[1 + 2 * i].content.second, block_graph[2 + 2 * i].content.second);
      }

    ASSERT_EQ(*block_graph[1].content.second, std::set<std::size_t>{1});

    std::set<std::size_t> cont{2, 3, 4};
    ASSERT_EQ(*block_graph[3].content.second, cont);

    ASSERT_EQ(*block_graph[5].content.second, std::set<std::size_t>{5});
    ASSERT_EQ(*block_graph[7].content.second, std::set<std::size_t>{6});
    ASSERT_EQ(*block_graph[9].content.second, std::set<std::size_t>{7});
  }

  TEST(BlockGraphBuilderTest, ConstructionClassic2)
  {
    auto graph  = basic_graph2(2);
    auto params = eppstein_parameters(5, 2);

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    for (std::size_t i = 0; i < 3; ++i)
      {
        ASSERT_EQ(*block_graph[1 + 2 * i].content.second, std::set<std::size_t>{i + 1});
        ASSERT_EQ(*block_graph[2 + 2 * i].content.second, std::set<std::size_t>{i + 1});
      }
  }


  TEST(BlockGraphBuilderTest, ConstructionInput)
  {
    auto graph              = basic_graph(2);
    auto params             = eppstein_parameters(5, 2);
    params.block_graph_mode = parameters::Block_Graph_Generation_Mode::input;

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    // Same device, same content
    for (std::size_t i = 0; i <= 2; ++i)
      {
        ASSERT_EQ(block_graph[1 + 2 * i].content.second, block_graph[2 + 2 * i].content.second);
      }

    std::set<std::size_t> cont{1, 2, 3, 4};
    ASSERT_EQ(*block_graph[1].content.second, cont);

    ASSERT_EQ(*block_graph[3].content.second, std::set<std::size_t>{5});

    ASSERT_EQ(*block_graph[5].content.second, std::set<std::size_t>{6});
    ASSERT_EQ(*block_graph[7].content.second, std::set<std::size_t>{7});
  }

  TEST(BlockGraphBuilderTest, ConstructionInput2)
  {
    auto graph              = basic_graph2(2);
    auto params             = eppstein_parameters(5, 2);
    params.block_graph_mode = parameters::Block_Graph_Generation_Mode::input;

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    // Same device, same content
    for (std::size_t i = 0; i <= 1; ++i)
      {
        ASSERT_EQ(block_graph[1 + 2 * i].content.second, block_graph[2 + 2 * i].content.second);
      }

    std::set<std::size_t> cont{1, 2};
    ASSERT_EQ(*block_graph[1].content.second, cont);
    ASSERT_EQ(*block_graph[3].content.second, std::set<std::size_t>{3});
    ASSERT_EQ(*block_graph[5].content.second, std::set<std::size_t>{4});
  }


  TEST(BlockGraphBuilderTest, ConstructionOutput)
  {
    auto graph              = basic_graph(2);
    auto params             = eppstein_parameters(5, 2);
    params.block_graph_mode = parameters::Block_Graph_Generation_Mode::output;

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    // Same device, same content
    for (std::size_t i = 0; i <= 2; ++i)
      {
        ASSERT_EQ(block_graph[1 + 2 * i].content.second, block_graph[2 + 2 * i].content.second);
      }

    ASSERT_EQ(*block_graph[1].content.second, std::set<std::size_t>{1});

    std::set<std::size_t> cont{2, 3, 4, 5};
    ASSERT_EQ(*block_graph[3].content.second, cont);

    ASSERT_EQ(*block_graph[5].content.second, std::set<std::size_t>{6});
    ASSERT_EQ(*block_graph[7].content.second, std::set<std::size_t>{7});
  }

  TEST(BlockGraphBuilderTest, ConstructionOutput2)
  {
    auto graph              = basic_graph2(2);
    auto params             = eppstein_parameters(5, 2);
    params.block_graph_mode = parameters::Block_Graph_Generation_Mode::output;

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    // Same device, same content
    for (std::size_t i = 0; i <= 1; ++i)
      {
        ASSERT_EQ(block_graph[1 + 2 * i].content.second, block_graph[2 + 2 * i].content.second);
      }

    ASSERT_EQ(*block_graph[1].content.second, std::set<std::size_t>{1});

    std::set<std::size_t> cont{2, 3};
    ASSERT_EQ(*block_graph[3].content.second, cont);
    ASSERT_EQ(*block_graph[5].content.second, std::set<std::size_t>{4});
  }


  TEST(BlockGraphBuilderTest, TransmissionWeights)
  {
    auto graph              = basic_graph(2);
    auto params             = eppstein_parameters(5, 2);
    params.block_graph_mode = parameters::Block_Graph_Generation_Mode::output;

    Constrained_Block_Graph_Builder builder(graph, params);
    builder.construct_transmission_weights([](node_id_type const &node_id, std::size_t in_dev, std::size_t out_dev) {
      if (in_dev == out_dev)
        return 1.;
      else if (in_dev > out_dev)
        return 0.;
      else
        return 2.;
    });

    auto const block_graph = builder.construct_block_graph();

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{0, 1}), 1.);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{0, 2}), 2.);

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{1, 3}), 1.);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{1, 4}), 2.);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{2, 3}), 0.);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{2, 4}), 1.);

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{3, 5}), 1.);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{3, 6}), 2.);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{4, 5}), 0.);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{4, 6}), 1.);
  }


  TEST(BlockGraphBuilderTest, WeightsBlockSingle)
  {
    auto graph                               = basic_graph(2);
    auto params                              = eppstein_parameters(5, 2);
    params.block_graph_mode                  = parameters::Block_Graph_Generation_Mode::output;
    params.weights_params.weight_import_mode = parameters::Weight_Import_Mode::block_single_direct_read;

    params.weights_params.separator = ',';

    params.weights_params.single_weight_import_path  = "test_data/weights/sample_weights.csv";
    params.weights_params.single_csv_columns_weights = {"fake_weight_1", "fake_weight_2"};


    Constrained_Block_Graph_Builder builder(graph, params);
    builder.construct_operation_weights();

    auto const block_graph = builder.construct_block_graph();

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{0, 1}), 1234.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{0, 2}), 1234.2);

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{1, 3}), 5.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{1, 4}), 5.2);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{2, 3}), 5.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{2, 4}), 5.2);

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{3, 5}), 6.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{3, 6}), 6.2);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{4, 5}), 6.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{4, 6}), 6.2);
  }

  TEST(BlockGraphBuilderTest, WeightsBlockMultiple)
  {
    auto graph                = basic_graph(2);
    auto params               = eppstein_parameters(5, 2);
    params.block_graph_mode                  = parameters::Block_Graph_Generation_Mode::output;
    params.weights_params.weight_import_mode = parameters::Weight_Import_Mode::block_multiple_direct_read;

    params.weights_params.separator = ',';

    params.devices[0].relevant_entry = "fake_weight_1";
    params.devices[0].weights_path   = "test_data/weights/sample_weights.csv";

    params.devices[1].relevant_entry = "fake_weight_1";
    params.devices[1].weights_path   = "test_data/weights/sample_weights2.csv";


    Constrained_Block_Graph_Builder builder(graph, params);
    builder.construct_operation_weights();

    auto const block_graph = builder.construct_block_graph();

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{0, 1}), 1234.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{0, 2}), 1234.3);

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{1, 3}), 5.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{1, 4}), 5.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{2, 3}), 5.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{2, 4}), 5.1);

    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{3, 5}), 6.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{3, 6}), 6.6);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{4, 5}), 6.1);
    ASSERT_FLOAT_EQ(block_graph.get_weight(std::pair{4, 6}), 6.6);
  }


  GraphType
  basic_graph(std::size_t num_devices)
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(Content_type({}, {{"X0", 0}}));
    nodes.emplace_back(Content_type({{"X0", 0}}, {{"X1", 1}}));
    nodes.emplace_back(Content_type({{"X1", 1}}, {{"X2", 2}}));
    nodes.emplace_back(Content_type({{"X1", 1}}, {{"X3", 3}}));
    nodes.emplace_back(Content_type({{"X3", 3}}, {{"X4", 4}}));
    nodes.emplace_back(Content_type({{"X2", 2}, {"X4", 4}}, {{"X5", 5}}));
    nodes.emplace_back(Content_type({{"X5", 5}}, {{"X6", 6}}));
    nodes.emplace_back(Content_type({{"X6", 6}}, {{"X7", 7}}));

    return GraphType(2, std::move(nodes));
  }

  GraphType
  basic_graph2(std::size_t num_devices)
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(Content_type({}, {{"X0", 0}}));
    nodes.emplace_back(Content_type({{"X0", 0}}, {{"X1", 1}}));
    nodes.emplace_back(Content_type({{"X1", 1}}, {{"X2", 2}}));
    nodes.emplace_back(Content_type({{"X1", 1}, {"X2", 2}}, {{"X3", 3}}));
    nodes.emplace_back(Content_type({{"X3", 3}}, {{"X4", 4}}));

    return GraphType(2, std::move(nodes));
  }

  std::function<type_weight(node_id_type const &, std::size_t, std::size_t)>
  basic_transmission(std::size_t devices, std::size_t size)
  {
    return [devices, size](node_id_type const &input, std::size_t first, std::size_t second) {
      if (0 <= input && input < size && first < devices && second < devices)
        {
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
        }
      else
        {
          std::cout << "Incorrect device id or input id" << std::endl;
          return -1.;
        }
    };
  }

  parameters::Parameters
  eppstein_parameters(std::size_t k, std::size_t num_devices)
  {
    parameters::Parameters res;

    res.K                            = 5;
    res.backward_connections_allowed = true;
    res.method                       = parameters::KSP_Method::Eppstein;
    res.devices                      = std::vector<parameters::Device>(num_devices);

    for (std::size_t i = 0; i < res.devices.size(); ++i)
      res.devices[i].id = i;

    res.memory_constraint_type = parameters::Memory_Constraint_Type::None;
    res.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;

    res.starting_device_id = 0;
    res.ending_device_id   = 0;

    return res;
  }

  parameters::Parameters
  lazy_eppstein_parameters(std::size_t k, std::size_t num_devices)
  {
    parameters::Parameters res;

    res.K                            = k;
    res.backward_connections_allowed = true;
    res.method                       = parameters::KSP_Method::Lazy_Eppstein;
    res.devices                      = std::vector<parameters::Device>(num_devices);

    for (std::size_t i = 0; i < res.devices.size(); ++i)
      res.devices[i].id = i;

    res.memory_constraint_type = parameters::Memory_Constraint_Type::None;
    res.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;

    res.starting_device_id = 0;
    res.ending_device_id   = 0;

    return res;
  }
} // namespace