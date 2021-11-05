//
// Created by faccus on 26/10/21.
//

#include "../../../src/Helpers/K-shortest_path/KEppstein.h"
#include "../../TestClass.h"
#include <gtest/gtest.h>

TEST(KspTests, Constructor)
{
  using basic_type    = int;
  using Input         = TestMemoryUsage<basic_type>;
  int number_of_nodes = 10;


  Graph<Input>                                            basic_graph;
  std::map<std::pair<node_id_type, node_id_type>, double> weight;

  KFinder kFinder(basic_graph);
  auto    res = kFinder.dijkstra(weight);

  ASSERT_EQ(res.first.size(), 0);
}


TEST(KspTests, DijkstraSourceSink)
{
  using basic_type = int;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, basic_type> map;
  std::vector<node_type>           nodes;

  nodes.push_back(node_type(0, {}, {0}, {}));
  nodes.push_back(node_type(1, {0, 2, 4}, {1}, {}));
  nodes.push_back(node_type(2, {0, 3}, {2}, {}));
  nodes.push_back(node_type(3, {1, 5}, {3}, {}));
  nodes.push_back(node_type(4, {2, 3, 6}, {4}, {}));
  nodes.push_back(node_type(5, {2, 4}, {5}, {}));
  nodes.push_back(node_type(6, {5}, {6}, {}));

  for (io_id_type i = 0; i <= 6; ++i)
    map[i] = i;

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


  Graph<basic_type> graph_cons(nodes, map);
  KFinder           kfinder(graph_cons);
  auto              res = kfinder.dijkstra(weights);

  std::vector<node_id_type> theoretical_res = {0, 2, 0, 1, 3, 2, 5};

  ASSERT_EQ(res.first, theoretical_res);
}


TEST(KspTests, DijkstraSinkSource)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.push_back(node_type(0, {}, {0}, {}));
  nodes.push_back(node_type(1, {0, 2, 4}, {1}, {}));
  nodes.push_back(node_type(2, {0, 3}, {2}, {}));
  nodes.push_back(node_type(3, {1, 5}, {3}, {}));
  nodes.push_back(node_type(4, {2, 3, 6}, {4}, {}));
  nodes.push_back(node_type(5, {2, 4}, {5}, {}));
  nodes.push_back(node_type(6, {5}, {6}, {}));

  for (io_id_type i = 0; i <= 6; ++i)
    map[i] = i;

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


  Graph<Input> graph_cons(nodes, map);
  KFinder      kfinder(graph_cons);
  auto         res = kfinder.dijkstra(weights, 6, true);

  std::vector<node_id_type> theoretical_res = {2, 3, 5, 4, 5, 6, 6};

  ASSERT_EQ(res.first, theoretical_res);
}


TEST(KspTests, DijkstraSourceSinkFunctional)
{
  using basic_type = int;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, basic_type> map;
  std::vector<node_type>           nodes;

  nodes.push_back(node_type(0, {}, {0}, {}));
  nodes.push_back(node_type(1, {0, 2, 4}, {1}, {}));
  nodes.push_back(node_type(2, {0, 3}, {2}, {}));
  nodes.push_back(node_type(3, {1, 5}, {3}, {}));
  nodes.push_back(node_type(4, {2, 3, 6}, {4}, {}));
  nodes.push_back(node_type(5, {2, 4}, {5}, {}));
  nodes.push_back(node_type(6, {5}, {6}, {}));

  for (io_id_type i = 0; i <= 6; ++i)
    map[i] = i;

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


  Graph<basic_type>                             graph_cons(nodes, map);
  std::function<type_weight(edge_type const &)> weight_fun =
    [&weights](edge_type const &edge) {
      auto const it = weights.find(edge);
      if (it == weights.cend())
        return -1.;
      else
        return it->second;
    };

  KFinder kfinder(graph_cons);
  auto    res = kfinder.dijkstra(weight_fun);

  std::vector<node_id_type> theoretical_res = {0, 2, 0, 1, 3, 2, 5};

  ASSERT_EQ(res.first, theoretical_res);
}


TEST(KspTests, DijkstraSinkSourceFunctional)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;

  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.push_back(node_type(0, {}, {0}, {}));
  nodes.push_back(node_type(1, {0, 2, 4}, {1}, {}));
  nodes.push_back(node_type(2, {0, 3}, {2}, {}));
  nodes.push_back(node_type(3, {1, 5}, {3}, {}));
  nodes.push_back(node_type(4, {2, 3, 6}, {4}, {}));
  nodes.push_back(node_type(5, {2, 4}, {5}, {}));
  nodes.push_back(node_type(6, {5}, {6}, {}));

  for (io_id_type i = 0; i <= 6; ++i)
    map[i] = i;

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


  Graph<Input>                                  graph_cons(nodes, map);
  std::function<type_weight(edge_type const &)> weight_fun =
    [&weights](edge_type const &edge) {
      auto const it = weights.find(edge);
      if (it == weights.cend())
        return -1.;
      else
        return it->second;
    };
  KFinder kfinder(graph_cons);
  auto    res = kfinder.dijkstra(weight_fun, 6, true);

  std::vector<node_id_type> theoretical_res = {2, 3, 5, 4, 5, 6, 6};

  ASSERT_EQ(res.first, theoretical_res);
}


TEST(KspTests, EppsteinOriginalNetwork)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;


  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.push_back(node_type(0, {}, {0}, {}));

  for (int i = 1; i < 12; ++i)
    {
      if (i < 4)
        nodes.push_back(node_type(i, {i - 1}, {i}, {}));
      else if (i == 4)
        nodes.push_back(node_type(4, {0}, {4}, {}));
      else if (4 < i && i < 8)
        nodes.push_back(node_type(i, {i % 4, i - 1}, {i}, {}));
      else if (i == 8)
        nodes.push_back(node_type(8, {4}, {8}, {}));
      else if (i > 8)
        nodes.push_back(node_type(i, {i % 4 + 4, i - 1}, {i}, {}));
    }

  for (io_id_type i = 0; i < 12; ++i)
    map[i] = i;

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


  Graph<Input>     graph_cons(nodes, map);
  KFinder_Eppstein kfinder(graph_cons);

  int k = 100; // Up to 10

  std::vector<type_weight> real_sol = {
    55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
  auto res = kfinder.eppstein(weights, real_sol.size());

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

TEST(KspTests, EppsteinLinearGraph)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;


  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.emplace_back(node_type(0, {}, {0}));
  nodes.emplace_back(node_type(1, {0}, {1}));
  nodes.emplace_back(node_type(2, {1, 4}, {2}));
  nodes.emplace_back(node_type(3, {2, 5}, {3}));


  for (io_id_type i = 0; i < nodes.size(); ++i)
    map[i] = i;

  type_collection_weights weights;
  weights[{0, 1}] = 1;
  weights[{0, 4}] = 0;
  weights[{1, 2}] = 1;
  weights[{1, 5}] = 1;
  weights[{2, 3}] = 1;
  weights[{4, 2}] = 2;
  weights[{4, 5}] = 1;
  weights[{5, 3}] = 0;

  Graph<Input>     graph_cons(nodes, map);
  KFinder_Eppstein kfinder(graph_cons);
  auto             res = kfinder.eppstein_linear(weights, 1000, 2);

  int                      k       = 10;
  std::vector<type_weight> lengths = {1, 2, 3, 3};

  std::vector<type_weight> real_path_lengths;
  std::vector<type_weight> path_lengths;

  path_lengths.reserve(k);
  real_path_lengths.reserve(k);

  for (auto i = 0; i < k && i < path_lengths.size(); ++i)
    {
      path_lengths.push_back(res[i].length);
      real_path_lengths.push_back(lengths[i]);
    }

  ASSERT_EQ(path_lengths, real_path_lengths);
}


TEST(KspTests, LazyEppsteinOriginalNetwork)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;


  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.push_back(node_type(0, {}, {0}, {}));

  for (int i = 1; i < 12; ++i)
    {
      if (i < 4)
        nodes.push_back(node_type(i, {i - 1}, {i}, {}));
      else if (i == 4)
        nodes.push_back(node_type(4, {0}, {4}, {}));
      else if (4 < i && i < 8)
        nodes.push_back(node_type(i, {i % 4, i - 1}, {i}, {}));
      else if (i == 8)
        nodes.push_back(node_type(8, {4}, {8}, {}));
      else if (i > 8)
        nodes.push_back(node_type(i, {i % 4 + 4, i - 1}, {i}, {}));
    }

  for (io_id_type i = 0; i < 12; ++i)
    map[i] = i;

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


  Graph<Input>     graph_cons(nodes, map);
  KFinder_Eppstein kfinder(graph_cons);

  int k = 100; // Up to 10

  std::vector<type_weight> real_sol = {
    55., 58., 59., 61., 62., 64., 65., 68., 68., 71.};
  auto res = kfinder.lazy_eppstein(weights, real_sol.size());

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

TEST(KspTests, LazyEppsteinLinearGraph)
{
  using basic_type = int;
  using Input      = TestMemoryUsage<basic_type>;

  using type_weight = double;
  using type_collection_weights =
    std::map<std::pair<node_id_type, node_id_type>, type_weight>;


  std::map<io_id_type, Input> map;
  std::vector<node_type>      nodes;

  nodes.emplace_back(node_type(0, {}, {0}));
  nodes.emplace_back(node_type(1, {0}, {1}));
  nodes.emplace_back(node_type(2, {1, 4}, {2}));
  nodes.emplace_back(node_type(3, {2, 5}, {3}));


  for (io_id_type i = 0; i < nodes.size(); ++i)
    map[i] = i;

  type_collection_weights weights;
  weights[{0, 1}] = 1;
  weights[{0, 4}] = 0;
  weights[{1, 2}] = 1;
  weights[{1, 5}] = 1;
  weights[{2, 3}] = 1;
  weights[{4, 2}] = 2;
  weights[{4, 5}] = 1;
  weights[{5, 3}] = 0;

  Graph<Input>     graph_cons(nodes, map);
  KFinder_Eppstein kfinder(graph_cons);
  auto             res = kfinder.lazy_eppstein_linear(weights, 1000, 2);

  int                      k       = 10;
  std::vector<type_weight> lengths = {1, 2, 3, 3};

  std::vector<type_weight> real_path_lengths;
  std::vector<type_weight> path_lengths;

  path_lengths.reserve(k);
  real_path_lengths.reserve(k);

  for (auto i = 0; i < k && i < path_lengths.size(); ++i)
    {
      path_lengths.push_back(res[i].length);
      real_path_lengths.push_back(lengths[i]);
    }

  ASSERT_EQ(path_lengths, real_path_lengths);
}