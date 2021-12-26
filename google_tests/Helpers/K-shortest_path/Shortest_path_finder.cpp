//
// Created by faccus on 26/10/21.
//

#include "../../../src/Helpers/K-shortest_path/KEppstein.h"
#include "../../../src/Helpers/K-shortest_path/KEppstein_lazy.h"
#include "../../TestClass.h"
#include <gtest/gtest.h>

namespace KspTestNamespace
{
  using basic_type  = int;
  using type_weight = double;


  using Input = TestMemoryUsage<basic_type>;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;


  Graph<basic_type>
  basic_graph();
  type_collection_weights
  basic_weights();

  Graph<Input>
  eppstein_graph();
  type_collection_weights
  eppstein_weights();


  TEST(KspTests, Constructor)
  {
    using basic_type    = int;
    using Input         = TestMemoryUsage<basic_type>;
    int number_of_nodes = 10;


    Graph<Input>                                            basic_graph;
    std::map<std::pair<node_id_type, node_id_type>, double> weight;

    Shortest_path_finder kFinder(basic_graph);
    auto                 res = kFinder.dijkstra(weight);

    ASSERT_EQ(res.first.size(), 0);
  }


  TEST(KspTests, DijkstraSourceSink)
  {
    auto const graph = basic_graph();

    Shortest_path_finder kfinder(graph);
    auto                 res = kfinder.dijkstra(basic_weights());

    std::vector<node_id_type> theoretical_res = {0, 2, 0, 1, 3, 2, 5};

    ASSERT_EQ(res.first, theoretical_res);
  }

  TEST(KspTests, DijkstraSinkSource)
  {
    auto const graph = basic_graph();

    Shortest_path_finder kfinder(graph);
    auto                 res = kfinder.dijkstra(basic_weights(), 6, true);

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
    auto res = kfinder.eppstein(eppstein_weights(), real_sol.size());

    std::vector<type_weight> real_path_lengths;
    std::vector<type_weight> path_lengths;

    path_lengths.reserve(k);
    real_path_lengths.reserve(k);

    for (auto i = 0; i < k && i < res.size(); ++i)
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

    int k = 100;

    std::vector<type_weight> real_sol = {
      55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
    auto res = kfinder.lazy_eppstein(eppstein_weights(), real_sol.size());

    std::vector<type_weight> real_path_lengths;
    std::vector<type_weight> path_lengths;

    path_lengths.reserve(k);
    real_path_lengths.reserve(k);

    for (auto i = 0; i < k && i < res.size(); ++i)
      {
        path_lengths.push_back(res[i].length);
        real_path_lengths.push_back(real_sol[i]);
      }

    ASSERT_EQ(path_lengths, real_path_lengths);
  }


  Graph<basic_type>
  basic_graph()
  {
    std::map<io_id_type, basic_type> map;
    std::vector<node_type>           nodes;

    nodes.push_back(node_type(io_id_collection_type{}, {{"Y", 0}}));
    nodes.push_back(node_type({{"X1", 0}, {"X2", 2}, {"X3", 4}}, {{"Y", 1}}));
    nodes.push_back(node_type({{"X1", 0}, {"X2", 3}}, {{"Y", {2}}}));
    nodes.push_back(node_type({{"X1", 1}, {"X2", 5}}, {{"Y", 3}}));
    nodes.push_back(node_type({{"X1", 2}, {"X2", 3}, {"X3", 6}}, {{"Y", 4}}));
    nodes.push_back(node_type({{"X1", 2}, {"X2", 4}}, {{"Y", 5}}));
    nodes.push_back(node_type({{"X", 5}}, {{"Y", 6}}));

    for (io_id_type i = 0; i <= 6; ++i)
      map[i] = i;

    return Graph(nodes, map);
  }
  type_collection_weights
  basic_weights()
  {
    type_collection_weights weights;
    weights[{0, 1}] = 4;
    weights[{0, 2}] = 1;
    weights[{1, 3}] = 3;
    weights[{2, 1}] = 2;
    weights[{2, 4}] = 9;
    weights[{2, 5}] = 4;
    weights[{3, 2}] = 1;
    weights[{3, 4}] = 2;
    weights[{4, 1}] = 0;
    weights[{4, 5}] = 1;
    weights[{5, 3}] = 1;
    weights[{5, 6}] = 2;
    weights[{6, 4}] = 2;

    return weights;
  }


  Graph<Input>
  eppstein_graph()
  {
    std::map<io_id_type, Input> map;
    std::vector<node_type>      nodes;

    nodes.push_back(node_type(io_id_collection_type{}, {{"Y", 0}}, {}));

    for (int i = 1; i < 12; ++i)
      {
        if (i < 4)
          nodes.push_back(node_type({{"X", i - 1}}, {{"Y", i}}));
        else if (i == 4)
          nodes.push_back(node_type({{"X", 0}}, {{"Y", 4}}));
        else if (4 < i && i < 8)
          nodes.push_back(
            node_type({{"X1", i % 4}, {"X2", i - 1}}, {{"Y", i}}));
        else if (i == 8)
          nodes.push_back(node_type({{"X", 4}}, {{"Y", 8}}));
        else if (i > 8)
          nodes.push_back(
            node_type({{"X1", i % 4 + 4}, {"X2", i - 1}}, {{"Y", i}}));
      }

    for (io_id_type i = 0; i < 12; ++i)
      map[i] = i;

    return Graph(nodes, map);
  }
  type_collection_weights
  eppstein_weights()
  {
    type_collection_weights weights;
    weights[{0, 1}]   = 2;
    weights[{1, 2}]   = 20;
    weights[{2, 3}]   = 14;
    weights[{0, 4}]   = 13;
    weights[{1, 5}]   = 27;
    weights[{2, 6}]   = 14;
    weights[{3, 7}]   = 15;
    weights[{4, 5}]   = 9;
    weights[{5, 6}]   = 10;
    weights[{6, 7}]   = 25;
    weights[{4, 8}]   = 15;
    weights[{5, 9}]   = 20;
    weights[{6, 10}]  = 12;
    weights[{7, 11}]  = 7;
    weights[{8, 9}]   = 18;
    weights[{9, 10}]  = 8;
    weights[{10, 11}] = 11;

    return weights;
  }
} // namespace KspTestNamespace