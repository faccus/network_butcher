//
// Created by faccus on 26/10/21.
//

#include "Graph_traits.h"
#include "KEppstein.h"
#include "KEppstein_lazy.h"

#include "TestClass.h"
#include "TestGraph.h"
#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;
  using namespace types;
  using namespace network_butcher::kfinder;

  using basic_type  = int;
  using type_weight = double;

  using Input         = TestMemoryUsage<basic_type>;
  using Content_input = types::Content<Input>;
  using Node_type     = types::CNode<types::Content<Input>>;
  using Graph_type    = types::WGraph<false, Node_type>;

  template <bool Reversed>
  using Weighted_Graph_type =
    Weighted_Graph<Graph_type, Reversed, Graph_type::Node_Type, Graph_type::Node_Collection_Type, weight_type>;

  Graph_type
  eppstein_graph();

  TestGraph<basic_type>
  test_graph();


  TEST(KFinderTest, EppsteinOriginalNetwork)
  {
    auto const       graph = eppstein_graph();
    KFinder_Eppstein kfinder(graph, graph.get_nodes().front().get_id(), graph.get_nodes().back().get_id());

    int k = 100; // Up to 10

    std::vector<type_weight> real_sol = {55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
    auto                     res      = kfinder.compute(real_sol.size());

    EXPECT_EQ(real_sol.size(), res.size());

    for (std::size_t j = 0; j < res.size(); ++j)
      {
        auto const &path = res[j];

        decltype(path.length) weight = 0.;
        for (std::size_t i = 0; i < path.path.size() - 1; ++i)
          {
            ASSERT_TRUE(graph.get_output_nodes(path.path[i]).contains(path.path[i + 1]));
            weight += graph.get_weight(std::make_pair(path.path[i], path.path[i + 1]));
          }

        ASSERT_EQ(graph.get_nodes().front().get_id(), path.path.front());
        ASSERT_EQ(graph.get_nodes().back().get_id(), path.path.back());

        ASSERT_EQ(weight, path.length);
        ASSERT_EQ(real_sol[j], path.length);
      }
  }

  TEST(KFinderTest, LazyEppsteinOriginalNetwork)
  {
    auto const            graph = eppstein_graph();
    KFinder_Lazy_Eppstein kfinder(graph, graph.get_nodes().front().get_id(), graph.get_nodes().back().get_id());

    int k = 100; // Up to 10

    std::vector<type_weight> real_sol = {55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
    auto                     res      = kfinder.compute(real_sol.size());

    std::vector<type_weight> real_path_lengths;
    std::vector<type_weight> path_lengths;

    path_lengths.reserve(k);
    real_path_lengths.reserve(k);

    EXPECT_EQ(real_sol.size(), res.size());

    for (std::size_t j = 0; j < res.size(); ++j)
      {
        auto const &path = res[j];

        decltype(path.length) weight = 0.;
        for (std::size_t i = 0; i < path.path.size() - 1; ++i)
          {
            ASSERT_TRUE(graph.get_output_nodes(path.path[i]).contains(path.path[i + 1]));
            weight += graph.get_weight(std::make_pair(path.path[i], path.path[i + 1]));
          }

        ASSERT_EQ(graph.get_nodes().front().get_id(), path.path.front());
        ASSERT_EQ(graph.get_nodes().back().get_id(), path.path.back());

        ASSERT_EQ(weight, path.length);
        ASSERT_EQ(real_sol[j], path.length);
      }
  }

  TEST(KFinderTest, LazyEppsteinOriginalTestGraph)
  {
    auto const            graph = test_graph();
    KFinder_Lazy_Eppstein kfinder(graph, 0, 11);

    int k = 100; // Up to 10

    std::vector<type_weight> real_sol = {55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
    auto                     res      = kfinder.compute(real_sol.size());

    std::vector<type_weight> real_path_lengths;
    std::vector<type_weight> path_lengths;

    path_lengths.reserve(k);
    real_path_lengths.reserve(k);

    EXPECT_EQ(real_sol.size(), res.size());

    for (std::size_t j = 0; j < res.size(); ++j)
      {
        auto const &path = res[j];

        decltype(path.length) weight = 0.;
        for (std::size_t i = 0; i < path.path.size() - 1; ++i)
          {
            ASSERT_TRUE(graph.dependencies.find(path.path[i])->second.second.contains(path.path[i + 1]));
            weight += graph.map_weight.find(std::make_pair(path.path[i], path.path[i + 1]))->second;
          }

        ASSERT_EQ(0, path.path.front());
        ASSERT_EQ(graph.nodes.size() - 1, path.path.back());

        ASSERT_EQ(weight, path.length);
        ASSERT_EQ(real_sol[j], path.length);
      }
  }

  Graph_type
  eppstein_graph()
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(std::move(Content_Builder<Input>().set_output({{"X0", 0}})).build());

    for (int i = 1; i < 12; ++i)
      {
        if (i < 4)
          {
            nodes.emplace_back(std::move(Content_Builder<Input>()
                                           .set_input({{"X" + std::to_string(i - 1), i - 1}})
                                           .set_output({{"X" + std::to_string(i), i}}))
                                 .build());
          }
        else if (i == 4)
          {
            nodes.emplace_back(
              std::move(Content_Builder<Input>().set_input({{"X0", 0}}).set_output({{"X4", 4}})).build());
          }
        else if (4 < i && i < 8)
          {
            nodes.emplace_back(
              std::move(Content_Builder<Input>()
                          .set_input({{"X" + std::to_string(i % 4), i % 4}, {"X" + std::to_string(i - 1), i - 1}})
                          .set_output({{"X" + std::to_string(i), i}}))
                .build());
          }
        else if (i == 8)
          {
            nodes.emplace_back(
              std::move(Content_Builder<Input>().set_input({{"X4", 4}}).set_output({{"X8", 8}})).build());
          }
        else if (i > 8)
          {
            nodes.emplace_back(std::move(Content_Builder<Input>()
                                           .set_input({{"X" + std::to_string(i % 4 + 4), i % 4 + 4},
                                                       {"X" + std::to_string(i - 1), i - 1}})
                                           .set_output({{"X" + std::to_string(i), i}}))
                                 .build());
          }
      }

    Graph_type graph(std::move(nodes));

    graph.set_weight({0, 1}, 2);
    graph.set_weight({1, 2}, 20);
    graph.set_weight({2, 3}, 14);
    graph.set_weight({0, 4}, 13);
    graph.set_weight({1, 5}, 27);
    graph.set_weight({2, 6}, 14);
    graph.set_weight({3, 7}, 15);
    graph.set_weight({4, 5}, 9);
    graph.set_weight({5, 6}, 10);
    graph.set_weight({6, 7}, 25);
    graph.set_weight({4, 8}, 15);
    graph.set_weight({5, 9}, 20);
    graph.set_weight({6, 10}, 12);
    graph.set_weight({7, 11}, 7);
    graph.set_weight({8, 9}, 18);
    graph.set_weight({9, 10}, 8);
    graph.set_weight({10, 11}, 11);

    return graph;
  }

  TestGraph<basic_type>
  test_graph()
  {
    using content_in = types::Content<Input>;

    auto const built_graph = eppstein_graph();

    TestGraph<basic_type> res;
    auto                 &nodes        = res.nodes;
    auto                 &dependencies = res.dependencies;
    auto                 &weights      = res.map_weight;

    for (auto const &node : built_graph.get_nodes())
      {
        nodes.push_back({node.get_id(), 90});
      }

    for (std::size_t i = 0; i < built_graph.size(); ++i)
      {
        dependencies[i] = std::make_pair(built_graph.get_input_nodes(i), built_graph.get_output_nodes(i));
      }

    weights.emplace(std::make_pair(0, 1), 2);
    weights.emplace(std::make_pair(1, 2), 20);
    weights.emplace(std::make_pair(2, 3), 14);
    weights.emplace(std::make_pair(0, 4), 13);
    weights.emplace(std::make_pair(1, 5), 27);
    weights.emplace(std::make_pair(2, 6), 14);
    weights.emplace(std::make_pair(3, 7), 15);
    weights.emplace(std::make_pair(4, 5), 9);
    weights.emplace(std::make_pair(5, 6), 10);
    weights.emplace(std::make_pair(6, 7), 25);
    weights.emplace(std::make_pair(4, 8), 15);
    weights.emplace(std::make_pair(5, 9), 20);
    weights.emplace(std::make_pair(6, 10), 12);
    weights.emplace(std::make_pair(7, 11), 7);
    weights.emplace(std::make_pair(8, 9), 18);
    weights.emplace(std::make_pair(9, 10), 8);
    weights.emplace(std::make_pair(10, 11), 11);


    return res;
  }
} // namespace