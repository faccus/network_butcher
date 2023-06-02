//
// Created by faccus on 01/05/23.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "test_class.h"

#include "constrained_block_graph_builder.h"
#include "graph_traits.h"

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;

  using type_weight             = double;
  using type_collection_weights = std::map<std::pair<Node_Id_Type, Node_Id_Type>, type_weight>;

  using basic_type   = int;
  using Input        = Test_Class<int>;
  using Content_type = Content<Input>;
  using Node_type    = CNode<Content_type>;
  using GraphType    = MWGraph<false, Node_type>;

  GraphType
  basic_graph(std::size_t num_devices = 2);

  GraphType
  basic_graph2(std::size_t num_devices = 2);


  parameters::Parameters
  full_connection_parameters();

  parameters::Parameters
  partial_connection_parameters();

  template <class Graph>
  void
  complete_weights(Graph &graph)
  {
    auto const  num_nodes    = graph.get_nodes().size();
    auto const &dependencies = graph.get_neighbors();

    for (Node_Id_Type tail = 0; tail < num_nodes; ++tail)
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
    auto graph  = basic_graph();
    auto params = full_connection_parameters();

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
    auto graph  = basic_graph();
    auto params = full_connection_parameters();

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
    auto graph  = basic_graph2();
    auto params = full_connection_parameters();

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
    auto graph                                            = basic_graph();
    auto params                                           = full_connection_parameters();
    params.block_graph_generation_params.block_graph_mode = parameters::Block_Graph_Generation_Mode::input;

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
    auto graph                                            = basic_graph2();
    auto params                                           = full_connection_parameters();
    params.block_graph_generation_params.block_graph_mode = parameters::Block_Graph_Generation_Mode::input;

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
    auto graph                                            = basic_graph();
    auto params                                           = full_connection_parameters();
    params.block_graph_generation_params.block_graph_mode = parameters::Block_Graph_Generation_Mode::output;

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
    auto graph                                            = basic_graph2();
    auto params                                           = full_connection_parameters();
    params.block_graph_generation_params.block_graph_mode = parameters::Block_Graph_Generation_Mode::output;

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


  TEST(BlockGraphBuilderTest, CheckNeighbours)
  {
    auto graph  = basic_graph();
    auto params = full_connection_parameters();

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    // Node 0 should be fully connected with the next one
    std::set<Node_Id_Type> tmp{1, 2};
    ASSERT_EQ(block_graph.get_output_nodes(0), tmp);

    // Nodes 1 and 2 should be connected with Node 0 (in), 3,4 (out)
    tmp = std::set<Node_Id_Type>{3, 4};
    ASSERT_EQ(block_graph.get_input_nodes(1), std::set<Node_Id_Type>{0});
    ASSERT_EQ(block_graph.get_input_nodes(2), std::set<Node_Id_Type>{0});
    ASSERT_EQ(block_graph.get_output_nodes(1), tmp);
    ASSERT_EQ(block_graph.get_output_nodes(2), tmp);


    // Nodes 3 and 4 should be connected with Nodes 1,2 (in), 5,6 (out)
    tmp = std::set<Node_Id_Type>{1, 2};
    ASSERT_EQ(block_graph.get_input_nodes(3), tmp);
    ASSERT_EQ(block_graph.get_input_nodes(4), tmp);

    tmp = std::set<Node_Id_Type>{5, 6};
    ASSERT_EQ(block_graph.get_output_nodes(3), tmp);
    ASSERT_EQ(block_graph.get_output_nodes(4), tmp);


    // Nodes 5 and 6 should be connected with Nodes 3,4 (in), 7,8 (out)
    tmp = std::set<Node_Id_Type>{3, 4};
    ASSERT_EQ(block_graph.get_input_nodes(5), tmp);
    ASSERT_EQ(block_graph.get_input_nodes(6), tmp);

    tmp = std::set<Node_Id_Type>{7, 8};
    ASSERT_EQ(block_graph.get_output_nodes(5), tmp);
    ASSERT_EQ(block_graph.get_output_nodes(6), tmp);

    // Nodes 7 and 8 should be connected with Nodes 5,6 (in), 9 (out)
    tmp = std::set<Node_Id_Type>{5, 6};
    ASSERT_EQ(block_graph.get_input_nodes(7), tmp);
    ASSERT_EQ(block_graph.get_input_nodes(8), tmp);

    tmp = std::set<Node_Id_Type>{9};
    ASSERT_EQ(block_graph.get_output_nodes(7), tmp);
    ASSERT_EQ(block_graph.get_output_nodes(8), tmp);

    // Node 9 should be connected with Nodes 7,8 (in)
    tmp = std::set<Node_Id_Type>{7, 8};
    ASSERT_EQ(block_graph.get_input_nodes(9), tmp);
  }

  TEST(BlockGraphBuilderTest, CheckNeighbours2)
  {
    auto graph  = basic_graph(4);
    auto params = partial_connection_parameters();

    auto const &bandwidth = params.weights_params.bandwidth;

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    // Check output neighbours for node 0
    std::set<Node_Id_Type> tmp{};
    for (auto const &neighbour : bandwidth->get_output_nodes(params.block_graph_generation_params.starting_device_id))
      {
        tmp.insert(neighbour + 1);
      }
    ASSERT_EQ(block_graph.get_output_nodes(0), tmp);


    // Check input neighbours for 1->4 nodes
    for (std::size_t i = 1; i < 5; ++i)
      {
        if (tmp.contains(i))
          {
            ASSERT_EQ(block_graph.get_input_nodes(i), std::set<Node_Id_Type>{0});
          }
        else
          {
            ASSERT_EQ(block_graph.get_input_nodes(i), std::set<Node_Id_Type>{});
          }
      }

    // Check input neighbours for the 5->16 nodes
    for (std::size_t i = 5; i < 17; ++i)
      {
        tmp.clear();
        for (auto const &neighbour : bandwidth->get_input_nodes(block_graph[i].content.first))
          {
            tmp.insert(1 + neighbour + ((i - 5) / 4) * 4);
          }
        ASSERT_EQ(block_graph.get_input_nodes(i), tmp);
      }

    // Check output neighbours for the 1->12 nodes
    for (std::size_t i = 1; i < 13; ++i)
      {
        tmp.clear();
        for (auto const &neighbour : bandwidth->get_output_nodes(block_graph[i].content.first))
          {
            tmp.insert(5 + neighbour + ((i - 1) / 4) * 4);
          }
        ASSERT_EQ(block_graph.get_output_nodes(i), tmp);
      }


    // Check input neighbours for node 17
    tmp.clear();
    for (auto const &neighbour : bandwidth->get_input_nodes(params.block_graph_generation_params.ending_device_id))
      {
        tmp.insert(13 + neighbour);
      }
    ASSERT_EQ(block_graph.get_input_nodes(17), tmp);

    // Check output neighbours for the 13->16 nodes
    for (std::size_t i = 13; i < 17; ++i)
      {
        if (tmp.contains(i))
          {
            ASSERT_EQ(block_graph.get_output_nodes(i), std::set<Node_Id_Type>{17});
          }
        else
          {
            ASSERT_EQ(block_graph.get_output_nodes(i), std::set<Node_Id_Type>{});
          }
      }
  }

  TEST(BlockGraphBuilderTest, CheckNeighbours3)
  {
    auto graph  = basic_graph(4);
    auto params = partial_connection_parameters();

    params.weights_params.in_bandwidth[std::make_pair(0, 2)] = std::make_pair(1., 0.);

    auto const &bandwidth = params.weights_params.bandwidth;

    Constrained_Block_Graph_Builder builder(graph, params);

    auto const block_graph = builder.construct_block_graph();

    // Check output neighbours for node 0
    std::set<Node_Id_Type> tmp{};
    for (auto const &neighbour : bandwidth->get_output_nodes(params.block_graph_generation_params.starting_device_id))
      {
        tmp.insert(neighbour + 1);
      }
    tmp.insert(3);
    ASSERT_EQ(block_graph.get_output_nodes(0), tmp);


    // Check input neighbours for 1->4 nodes
    for (std::size_t i = 1; i < 5; ++i)
      {
        if (tmp.contains(i))
          {
            ASSERT_EQ(block_graph.get_input_nodes(i), std::set<Node_Id_Type>{0});
          }
        else
          {
            ASSERT_EQ(block_graph.get_input_nodes(i), std::set<Node_Id_Type>{});
          }
      }

    // Check input neighbours for the 5->16 nodes
    for (std::size_t i = 5; i < 17; ++i)
      {
        tmp.clear();
        for (auto const &neighbour : bandwidth->get_input_nodes(block_graph[i].content.first))
          {
            tmp.insert(1 + neighbour + ((i - 5) / 4) * 4);
          }
        ASSERT_EQ(block_graph.get_input_nodes(i), tmp);
      }

    // Check output neighbours for the 1->12 nodes
    for (std::size_t i = 1; i < 13; ++i)
      {
        tmp.clear();
        for (auto const &neighbour : bandwidth->get_output_nodes(block_graph[i].content.first))
          {
            tmp.insert(5 + neighbour + ((i - 1) / 4) * 4);
          }
        ASSERT_EQ(block_graph.get_output_nodes(i), tmp);
      }


    // Check input neighbours for node 17
    tmp.clear();
    ;
    for (auto const &neighbour : bandwidth->get_input_nodes(params.block_graph_generation_params.ending_device_id))
      {
        tmp.insert(13 + neighbour);
      }
    ASSERT_EQ(block_graph.get_input_nodes(17), tmp);

    // Check output neighbours for the 13->16 nodes
    for (std::size_t i = 13; i < 17; ++i)
      {
        if (tmp.contains(i))
          {
            ASSERT_EQ(block_graph.get_output_nodes(i), std::set<Node_Id_Type>{17});
          }
        else
          {
            ASSERT_EQ(block_graph.get_output_nodes(i), std::set<Node_Id_Type>{});
          }
      }
  }


  TEST(BlockGraphBuilderTest, TransmissionWeights)
  {
    auto graph                                            = basic_graph();
    auto params                                           = full_connection_parameters();
    params.block_graph_generation_params.block_graph_mode = parameters::Block_Graph_Generation_Mode::output;

    Constrained_Block_Graph_Builder builder(graph, params);
    builder.construct_transmission_weights([](Edge_Type const &node_id, std::size_t in_dev, std::size_t out_dev) {
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
    auto graph                                            = basic_graph();
    auto params                                           = full_connection_parameters();
    params.block_graph_generation_params.block_graph_mode = parameters::Block_Graph_Generation_Mode::output;
    params.weights_params.weight_import_mode              = parameters::Weight_Import_Mode::block_single_direct_read;

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
    auto graph                                            = basic_graph();
    auto params                                           = full_connection_parameters();
    params.block_graph_generation_params.block_graph_mode = parameters::Block_Graph_Generation_Mode::output;
    params.weights_params.weight_import_mode              = parameters::Weight_Import_Mode::block_multiple_direct_read;

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

    nodes.emplace_back(std::move(Content_Builder<Input>().set_output({{"X0", 0}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X0", 0}}).set_output({{"X1", 1}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X1", 1}}).set_output({{"X2", 2}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X1", 1}}).set_output({{"X3", 3}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X3", 3}}).set_output({{"X4", 4}})).build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X2", 2}, {"X4", 4}}).set_output({{"X5", 5}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X5", 5}}).set_output({{"X6", 6}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X6", 6}}).set_output({{"X7", 7}})).build());

    return GraphType(num_devices, std::move(nodes));
  }

  GraphType
  basic_graph2(std::size_t num_devices)
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(std::move(Content_Builder<Input>().set_output({{"X0", 0}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X0", 0}}).set_output({{"X1", 1}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X1", 1}}).set_output({{"X2", 2}})).build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X1", 1}, {"X2", 2}}).set_output({{"X3", 3}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X3", 3}}).set_output({{"X4", 4}})).build());

    return GraphType(num_devices, std::move(nodes));
  }


  std::function<type_weight(Edge_Type const &, std::size_t, std::size_t)>
  basic_transmission(std::size_t devices, std::size_t size)
  {
    return [devices, size](Edge_Type const &t_input, std::size_t first, std::size_t second) {
      auto const &[input, _tmp] = t_input;
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
  full_connection_parameters()
  {
    parameters::Parameters res;

    res.ksp_params.K      = 5;
    res.ksp_params.method = parameters::KSP_Method::Eppstein;
    res.devices           = std::vector<parameters::Device>(2);

    for (std::size_t i = 0; i < res.devices.size(); ++i)
      res.devices[i].id = i;

    res.block_graph_generation_params.memory_constraint_type = parameters::Memory_Constraint_Type::None;
    res.block_graph_generation_params.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;
    res.block_graph_generation_params.use_bandwidth_to_manage_connections = false;

    res.block_graph_generation_params.starting_device_id = 0;
    res.block_graph_generation_params.ending_device_id   = 0;

    return res;
  }

  parameters::Parameters
  partial_connection_parameters()
  {
    using g_type = parameters::Parameters::Weights::connection_type::element_type;
    parameters::Parameters res;

    res.ksp_params.K      = 5;
    res.ksp_params.method = parameters::KSP_Method::Lazy_Eppstein;
    res.devices           = std::vector<parameters::Device>(4);

    for (std::size_t i = 0; i < res.devices.size(); ++i)
      res.devices[i].id = i;

    res.block_graph_generation_params.memory_constraint_type = parameters::Memory_Constraint_Type::None;
    res.block_graph_generation_params.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;
    res.block_graph_generation_params.use_bandwidth_to_manage_connections = true;

    g_type::Dependencies_Type deps(4);
    deps[0] = std::make_pair(std::set<Node_Id_Type>{0}, std::set<Node_Id_Type>{0, 1, 3});
    deps[1] = std::make_pair(std::set<Node_Id_Type>{0, 1, 2}, std::set<Node_Id_Type>{1, 2});
    deps[2] = std::make_pair(std::set<Node_Id_Type>{1, 2}, std::set<Node_Id_Type>{1, 2, 3});
    deps[3] = std::make_pair(std::set<Node_Id_Type>{0, 2, 3}, std::set<Node_Id_Type>{3});

    res.weights_params.bandwidth = std::make_unique<g_type>(g_type::Node_Collection_Type(4), std::move(deps));


    res.block_graph_generation_params.starting_device_id = 0;
    res.block_graph_generation_params.ending_device_id   = 3;

    return res;
  }
} // namespace