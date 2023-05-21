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
  using Weighted_Graph_type = Weighted_Graph<Graph_type,
                                             Graph_type::Node_Type,
                                             Graph_type::Node_Collection_Type,
                                             Graph_type::Dependencies_Type,
                                             Reversed>;

  using weights_collection_type = std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  Graph_type
  basic_graph();

  Graph_type
  eppstein_graph();

  TestGraph<basic_type>
  test_graph();


  TEST(KspTests, DijkstraSourceSink)
  {
    auto const graph = basic_graph();

    auto res = Shortest_path_finder::dijkstra(Weighted_Graph_type<false>(graph));

    std::vector<node_id_type> theoretical_res = {0, 2, 0, 1, 3, 2, 5};

    ASSERT_EQ(res.first, theoretical_res);
  }

  TEST(KspTests, DijkstraSinkSource)
  {
    auto const graph = basic_graph();

    auto res = Shortest_path_finder::dijkstra(Weighted_Graph_type<true>(graph), 6);

    std::vector<node_id_type> theoretical_res = {2, 3, 5, 4, 5, 6, 6};

    ASSERT_EQ(res.first, theoretical_res);
  }


  TEST(KspTests, EppsteinOriginalNetwork)
  {
    auto const       graph = eppstein_graph();
    KFinder_Eppstein kfinder(graph, graph.get_nodes().front().get_id(), graph.get_nodes().back().get_id());

    int k = 100; // Up to 10

    std::vector<type_weight> real_sol = {55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
    auto                     res      = kfinder.compute(real_sol.size());

    std::vector<type_weight> real_path_lengths;
    std::vector<type_weight> path_lengths;

    path_lengths.reserve(k);
    real_path_lengths.reserve(k);

    EXPECT_EQ(real_sol.size(), res.size());

    for (auto i = 0; i < real_sol.size(); ++i)
      {
        path_lengths.push_back(res[i].length);
        real_path_lengths.push_back(real_sol[i]);
      }

    ASSERT_EQ(path_lengths, real_path_lengths);
  }

  TEST(KspTests, LazyEppsteinOriginalNetwork)
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

    for (auto i = 0; i < k && i < res.size(); ++i)
      {
        path_lengths.push_back(res[i].length);
        real_path_lengths.push_back(real_sol[i]);
      }

    ASSERT_EQ(path_lengths, real_path_lengths);
  }

  TEST(KspTests, LazyEppsteinOriginalTestGraph)
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

    for (auto i = 0; i < k && i < res.size(); ++i)
      {
        path_lengths.push_back(res[i].length);
        real_path_lengths.push_back(real_sol[i]);
      }

    ASSERT_EQ(path_lengths, real_path_lengths);
  }

  Graph_type
  basic_graph()
  {
    using content_in = types::Content<Input>;

    std::vector<Node_type> nodes;

    nodes.emplace_back(content_in(io_collection_type<Input>{}, io_collection_type<Input>{{"X0", 0}}));
    nodes.emplace_back(content_in({{"X0", 0}, {"X2", 2}, {"X4", 4}}, {{"X1", 1}}));
    nodes.emplace_back(content_in({{"X0", 0}, {"X3", 3}}, {{"X2", {2}}}));
    nodes.emplace_back(content_in({{"X1", 1}, {"X5", 5}}, {{"X3", 3}}));
    nodes.emplace_back(content_in({{"X2", 2}, {"X3", 3}, {"X6", 6}}, {{"X4", 4}}));
    nodes.emplace_back(content_in({{"X2", 2}, {"X4", 4}}, {{"X5", 5}}));
    nodes.emplace_back(content_in({{"X5", 5}}, {{"X6", 6}}));

    Graph_type graph(nodes);

    graph.set_weight({0, 1}, 4);
    graph.set_weight({0, 2}, 1);
    graph.set_weight({1, 3}, 3);
    graph.set_weight({2, 1}, 2);
    graph.set_weight({2, 4}, 9);
    graph.set_weight({2, 5}, 4);
    graph.set_weight({3, 2}, 1);
    graph.set_weight({3, 4}, 2);
    graph.set_weight({4, 1}, 0);
    graph.set_weight({4, 5}, 1);
    graph.set_weight({5, 3}, 1);
    graph.set_weight({5, 6}, 2);
    graph.set_weight({6, 4}, 2);

    return graph;
  }

  Graph_type
  eppstein_graph()
  {
    using content_in = types::Content<Input>;
    std::vector<Node_type> nodes;

    nodes.emplace_back(content_in({}, {{"X0", 0}}));

    for (int i = 1; i < 12; ++i)
      {
        if (i < 4)
          nodes.emplace_back(content_in({{"X" + std::to_string(i - 1), i - 1}}, {{"X" + std::to_string(i), i}}));
        else if (i == 4)
          nodes.emplace_back(content_in({{"X0", 0}}, {{"X4", 4}}));
        else if (4 < i && i < 8)
          nodes.emplace_back(content_in({{"X" + std::to_string(i % 4), i % 4}, {"X" + std::to_string(i - 1), i - 1}},
                                        {{"X" + std::to_string(i), i}}));
        else if (i == 8)
          nodes.emplace_back(content_in({{"X4", 4}}, {{"X8", 8}}));
        else if (i > 8)
          nodes.emplace_back(
            content_in({{"X" + std::to_string(i % 4 + 4), i % 4 + 4}, {"X" + std::to_string(i - 1), i - 1}},
                       {{"X" + std::to_string(i), i}}));
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
