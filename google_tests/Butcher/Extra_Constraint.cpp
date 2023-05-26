//
// Created by faccus on 22/05/23.
//
#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "TestClass.h"

#include "Constrained_Block_Graph_Builder.h"
#include "Graph_traits.h"

#include "Extra_Constraint.h"

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;

  using type_weight             = double;
  using type_collection_weights = std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  using basic_type   = int;
  using Input        = TestMemoryUsage<int>;
  using Content_type = Content<Input>;
  using Node_type    = CNode<Content_type>;
  using GraphType    = MWGraph<false, Node_type>;

  parameters::Parameters
  eppstein_parameters();

  parameters::Parameters
  lazy_eppstein_parameters();

  GraphType basic_graph(std::size_t);


  TEST(ExtraConstraintTest, BandwidthConstraint)
  {
    auto graph  = basic_graph(2);
    auto params = eppstein_parameters();

    Constrained_Block_Graph_Builder builder(graph, params);
    builder.add_constraint(std::make_unique<network_butcher::constraints::Bandwidth_Constraint>(params));

    auto res = builder.construct_block_graph();

    ASSERT_EQ(res.get_output_nodes(0), std::set<node_id_type>{1});

    ASSERT_EQ(res.get_output_nodes(1), std::set<node_id_type>{3});
    ASSERT_EQ(res.get_output_nodes(2), std::set<node_id_type>{4});
    ASSERT_EQ(res.get_input_nodes(2), std::set<node_id_type>{});

    ASSERT_EQ(res.get_output_nodes(3), std::set<node_id_type>{5});
    ASSERT_EQ(res.get_output_nodes(4), std::set<node_id_type>{6});

    ASSERT_EQ(res.get_output_nodes(5), std::set<node_id_type>{7});
    ASSERT_EQ(res.get_output_nodes(6), std::set<node_id_type>{8});

    ASSERT_EQ(res.get_output_nodes(7), std::set<node_id_type>{9});
    ASSERT_EQ(res.get_output_nodes(8), std::set<node_id_type>{});
  }


  TEST(ExtraConstraintTest, BandwidthConstraint2)
  {
    auto graph  = basic_graph(4);
    auto params = lazy_eppstein_parameters();

    Constrained_Block_Graph_Builder builder(graph, params);
    builder.add_constraint(std::make_unique<network_butcher::constraints::Bandwidth_Constraint>(params));

    auto res = builder.construct_block_graph();

    auto tmp = std::set<node_id_type>{1, 2, 4};
    ASSERT_EQ(res.get_output_nodes(0), tmp);
    ASSERT_EQ(res.get_input_nodes(3), std::set<node_id_type>{});

    for (node_id_type j = 1; j < 13; j += 4)
      {
        tmp = std::set<node_id_type>{4 + j, 5 + j, 7 + j};
        ASSERT_EQ(res.get_output_nodes(j), tmp);

        tmp = std::set<node_id_type>{5 + j, 6 + j};
        ASSERT_EQ(res.get_output_nodes(1 + j), tmp);

        tmp = std::set<node_id_type>{5 + j, 6 + j, 7 + j};
        ASSERT_EQ(res.get_output_nodes(2 + j), tmp);

        ASSERT_EQ(res.get_output_nodes(3 + j), std::set<node_id_type>{7 + j});
      }

    ASSERT_EQ(res.get_output_nodes(13), std::set<node_id_type>{17});
    ASSERT_EQ(res.get_output_nodes(14), std::set<node_id_type>{});
    ASSERT_EQ(res.get_output_nodes(15), std::set<node_id_type>{17});
    ASSERT_EQ(res.get_output_nodes(16), std::set<node_id_type>{17});

    tmp = std::set<node_id_type>{13, 15, 16};
    ASSERT_EQ(res.get_input_nodes(17), tmp);
  }


  GraphType
  basic_graph(std::size_t dev)
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

    return GraphType(dev, std::move(nodes));
  }

  parameters::Parameters
  eppstein_parameters()
  {
    parameters::Parameters res;

    res.ksp_params.K                            = 5;
    res.block_graph_generation_params.backward_connections_allowed = true;
    res.ksp_params.method                       = parameters::KSP_Method::Eppstein;
    res.devices                      = std::vector<parameters::Device>(2);

    for (std::size_t i = 0; i < res.devices.size(); ++i)
      res.devices[i].id = i;

    res.block_graph_generation_params.memory_constraint_type = parameters::Memory_Constraint_Type::None;
    res.block_graph_generation_params.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;

    res.block_graph_generation_params.starting_device_id = 0;
    res.block_graph_generation_params.ending_device_id   = 0;

    return res;
  }

  parameters::Parameters
  lazy_eppstein_parameters()
  {
    parameters::Parameters res;

    res.ksp_params.K                            = 5;
    res.block_graph_generation_params.backward_connections_allowed = true;
    res.ksp_params.method                       = parameters::KSP_Method::Lazy_Eppstein;
    res.devices                      = std::vector<parameters::Device>(4);

    for (std::size_t i = 0; i < res.devices.size(); ++i)
      res.devices[i].id = i;

    auto &band   = res.weights_params.bandwidth;
    band[{0, 1}] = std::make_pair(1., 0.);
    band[{0, 3}] = std::make_pair(2., 0.);

    band[{1, 2}] = std::make_pair(3., 0.);
    band[{2, 1}] = std::make_pair(4., 0.);

    band[{2, 3}] = std::make_pair(5., 0.);


    res.block_graph_generation_params.memory_constraint_type = parameters::Memory_Constraint_Type::None;
    res.block_graph_generation_params.block_graph_mode       = parameters::Block_Graph_Generation_Mode::classic;

    res.block_graph_generation_params.starting_device_id = 0;
    res.block_graph_generation_params.ending_device_id   = 3;

    return res;
  }
} // namespace