//
// Created by faccus on 26/10/21.
//

#include "../../../include/Helpers/K-shortest_path/KEppstein.h"
#include "../../../include/Helpers/K-shortest_path/KEppstein_lazy.h"

#include "../../TestClass.h"
#include <gtest/gtest.h>

namespace KspTestNamespace
{
  using basic_type  = int;
  using type_weight = double;

  using Input         = TestMemoryUsage<basic_type>;
  using Content_input = Content<Input>;
  using Node_type     = Node<Content<Input>>;

  using weights_collection_type =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;


  WGraph<Content_input>
  basic_graph();

  WGraph<Content_input>
  eppstein_graph();


  TEST(KspTests, Constructor)
  {
    using basic_type    = int;
    using Input         = TestMemoryUsage<basic_type>;
    int number_of_nodes = 10;


    WGraph<Input> basic_graph;

    Shortest_path_finder kFinder(basic_graph);
    auto                 res = kFinder.dijkstra();

    ASSERT_EQ(res.first.size(), 0);
  }


  TEST(KspTests, DijkstraSourceSink)
  {
    auto const graph = basic_graph();

    Shortest_path_finder kfinder(graph);
    auto                 res = kfinder.dijkstra();

    std::vector<node_id_type> theoretical_res = {0, 2, 0, 1, 3, 2, 5};

    ASSERT_EQ(res.first, theoretical_res);
  }

  TEST(KspTests, DijkstraSinkSource)
  {
    auto const graph = basic_graph();

    Shortest_path_finder kfinder(graph);
    auto                 res = kfinder.dijkstra(6, true);

    std::vector<node_id_type> theoretical_res = {2, 3, 5, 4, 5, 6, 6};

    ASSERT_EQ(res.first, theoretical_res);
  }


  TEST(KspTests, EppsteinOriginalNetwork)
  {
    auto const       graph = eppstein_graph();
    KFinder_Eppstein kfinder(graph);

    int k = 100; // Up to 10

    std::vector<type_weight> real_sol = {
      55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
    auto res = kfinder.compute(real_sol.size());

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
    KFinder_Lazy_Eppstein kfinder(graph);

    int k = 100; // Up to 10

    std::vector<type_weight> real_sol = {
      55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
    auto res = kfinder.compute(real_sol.size());

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


  WGraph<Content_input>
  basic_graph()
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(Content<Input>(io_collection_type<Input>{},
                                      io_collection_type<Input>{{"X0", 0}}));
    nodes.emplace_back(
      Content<Input>({{"X0", 0}, {"X2", 2}, {"X4", 4}}, {{"X1", 1}}));
    nodes.emplace_back(Content<Input>({{"X0", 0}, {"X3", 3}}, {{"X2", {2}}}));
    nodes.emplace_back(Content<Input>({{"X1", 1}, {"X5", 5}}, {{"X3", 3}}));
    nodes.emplace_back(
      Content<Input>({{"X2", 2}, {"X3", 3}, {"X6", 6}}, {{"X4", 4}}));
    nodes.emplace_back(Content<Input>({{"X2", 2}, {"X4", 4}}, {{"X5", 5}}));
    nodes.emplace_back(Content<Input>({{"X5", 5}}, {{"X6", 6}}));

    WGraph<Content_input> graph(nodes);

    graph.set_weigth({0, 1}, 4);
    graph.set_weigth({0, 2}, 1);
    graph.set_weigth({1, 3}, 3);
    graph.set_weigth({2, 1}, 2);
    graph.set_weigth({2, 4}, 9);
    graph.set_weigth({2, 5}, 4);
    graph.set_weigth({3, 2}, 1);
    graph.set_weigth({3, 4}, 2);
    graph.set_weigth({4, 1}, 0);
    graph.set_weigth({4, 5}, 1);
    graph.set_weigth({5, 3}, 1);
    graph.set_weigth({5, 6}, 2);
    graph.set_weigth({6, 4}, 2);

    return graph;
  }


  WGraph<Content_input>
  eppstein_graph()
  {
    std::vector<Node_type> nodes;

    nodes.emplace_back(Content<Input>({}, {{"X0", 0}}));

    for (int i = 1; i < 12; ++i)
      {
        if (i < 4)
          nodes.emplace_back(
            Content<Input>({{"X" + std::to_string(i - 1), i - 1}},
                           {{"X" + std::to_string(i), i}}));
        else if (i == 4)
          nodes.emplace_back(Content<Input>({{"X0", 0}}, {{"X4", 4}}));
        else if (4 < i && i < 8)
          nodes.emplace_back(
            Content<Input>({{"X" + std::to_string(i % 4), i % 4},
                            {"X" + std::to_string(i - 1), i - 1}},
                           {{"X" + std::to_string(i), i}}));
        else if (i == 8)
          nodes.emplace_back(Content<Input>({{"X4", 4}}, {{"X8", 8}}));
        else if (i > 8)
          nodes.emplace_back(
            Content<Input>({{"X" + std::to_string(i % 4 + 4), i % 4 + 4},
                            {"X" + std::to_string(i - 1), i - 1}},
                           {{"X" + std::to_string(i), i}}));
      }

    WGraph<Content_input> graph(std::move(nodes));

    graph.set_weigth({0, 1}, 2);
    graph.set_weigth({1, 2}, 20);
    graph.set_weigth({2, 3}, 14);
    graph.set_weigth({0, 4}, 13);
    graph.set_weigth({1, 5}, 27);
    graph.set_weigth({2, 6}, 14);
    graph.set_weigth({3, 7}, 15);
    graph.set_weigth({4, 5}, 9);
    graph.set_weigth({5, 6}, 10);
    graph.set_weigth({6, 7}, 25);
    graph.set_weigth({4, 8}, 15);
    graph.set_weigth({5, 9}, 20);
    graph.set_weigth({6, 10}, 12);
    graph.set_weigth({7, 11}, 7);
    graph.set_weigth({8, 9}, 18);
    graph.set_weigth({9, 10}, 8);
    graph.set_weigth({10, 11}, 11);

    return graph;
  }
} // namespace KspTestNamespace
