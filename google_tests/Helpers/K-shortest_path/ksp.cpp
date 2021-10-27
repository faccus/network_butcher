//
// Created by faccus on 26/10/21.
//

#include "../../../src/Helpers/K-shortest_path/ksp.h"
#include "../../TestClass.h"
#include <gtest/gtest.h>

TEST(KspTests, Constructor)
{
  using basic_type    = int;
  using Input         = TestMemoryUsage<basic_type>;
  int number_of_nodes = 10;


  Graph<Input>                                            basic_graph;
  std::map<std::pair<node_id_type, node_id_type>, double> weight;

  KFinder kFinder(basic_graph, weight);
  auto    res = kFinder.dijkstra();

  ASSERT_EQ(res.first.size(), 0);
}

TEST(KspTests, DijkstraSourceSink)
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
  nodes.push_back(node_type(4, {2, 3, 5, 6}, {4}, {}));
  nodes.push_back(node_type(5, {2}, {5}, {}));
  nodes.push_back(node_type(6, {5}, {6}, {}));

  for (io_id_type i = 1; i <= 7; ++i)
    map[i] = Input(i + 1);

  type_collection_weights weights;
  weights[{0, 1}] = 4;
  weights[{0, 2}] = 1;
  weights[{2, 1}] = 2;
  weights[{4, 1}] = 0;
  weights[{1, 3}] = 3;
  weights[{2, 4}] = 9;
  weights[{3, 2}] = 1;
  weights[{2, 5}] = 4;
  weights[{3, 4}] = 2;
  weights[{5, 4}] = 1;
  weights[{5, 6}] = 2;
  weights[{6, 4}] = 2;
  weights[{5, 3}] = 1;


  Graph<Input> graph_cons(nodes, map);
  KFinder      kfinder(graph_cons, weights);
  auto         res = kfinder.dijkstra();

  std::vector<node_id_type> theoretical_res = {0, 2, 0, 1, 5, 2, 5};

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
  nodes.push_back(node_type(4, {2, 3, 5, 6}, {4}, {}));
  nodes.push_back(node_type(5, {2}, {5}, {}));
  nodes.push_back(node_type(6, {5}, {6}, {}));

  for (io_id_type i = 1; i <= 7; ++i)
    map[i] = Input(i + 1);

  type_collection_weights weights;
  weights[{0, 1}] = 4;
  weights[{0, 2}] = 1;
  weights[{2, 1}] = 2;
  weights[{4, 1}] = 0;
  weights[{1, 3}] = 3;
  weights[{2, 4}] = 9;
  weights[{3, 2}] = 1;
  weights[{2, 5}] = 4;
  weights[{3, 4}] = 2;
  weights[{5, 4}] = 1;
  weights[{5, 6}] = 2;
  weights[{6, 4}] = 2;
  weights[{5, 3}] = 1;


  Graph<Input> graph_cons(nodes, map);
  KFinder      kfinder(graph_cons, weights);
  auto         res = kfinder.dijkstra(6, true);

  std::vector<node_id_type> theoretical_res = {2, 3, 5, 2, 1, 6, 6};

  ASSERT_EQ(res.first, theoretical_res);
}
