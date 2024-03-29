#include <network_butcher/Network/graph_traits.h>

#include <network_butcher/K-shortest_path/shortest_path_finder.h>

#include "../test_class.h"
#include "../test_graph.h"
#include <gtest/gtest.h>

/// Checks if the shortest path finder works correctly

namespace
{
  using namespace network_butcher;
  using namespace types;
  using namespace network_butcher::kfinder;

  using basic_type  = int;
  using type_weight = double;

  using Input         = Test_Class<basic_type>;
  using Node_type     = types::CNode<types::Content<Input>>;
  using Graph_type    = types::WGraph<false, Node_type>;

  template <bool Reversed>
  using Weighted_Graph_type =
    Weighted_Graph<Graph_type, Reversed, Graph_type::Node_Type, Graph_type::Node_Collection_Type, Time_Type>;

  auto
  basic_graph() -> Graph_type;

  auto
  eppstein_graph() -> Graph_type;

  auto
  test_graph() -> Test_Graph<basic_type>;

  /// Checks if Dijkstra works correctly
  TEST(ShortestPathFinderTest, DijkstraSourceSink)
  {
    auto const graph = basic_graph();

    auto res = Shortest_path_finder::dijkstra(Weighted_Graph_type<false>(graph), 0);

    std::vector<Node_Id_Type> theoretical_res = {0, 2, 0, 1, 3, 2, 5};

    ASSERT_EQ(res.first, theoretical_res);
  }

  /// Checks if Dijkstra works correctly
  TEST(ShortestPathFinderTest, DijkstraSinkSource)
  {
    auto const graph = basic_graph();

    auto res = Shortest_path_finder::dijkstra(Weighted_Graph_type<true>(graph), 6);

    std::vector<Node_Id_Type> theoretical_res = {2, 3, 5, 4, 5, 6, 6};

    ASSERT_EQ(res.first, theoretical_res);
  }

  auto
  basic_graph() -> Graph_type
  {
    std::vector<Node_type> nodes;


    nodes.emplace_back(std::move(Content_Builder<Input>().set_output(Io_Collection_Type<Input>{{"X0", 0}})).build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X0", 0}, {"X2", 2}, {"X4", 4}}).set_output({{"X1", 1}})).build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X0", 0}, {"X3", 3}}).set_output({{"X2", {2}}})).build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X1", 1}, {"X5", 5}}).set_output({{"X3", 3}})).build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X2", 2}, {"X3", 3}, {"X6", 6}}).set_output({{"X4", 4}})).build());
    nodes.emplace_back(
      std::move(Content_Builder<Input>().set_input({{"X2", 2}, {"X4", 4}}).set_output({{"X5", 5}})).build());
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X5", 5}}).set_output({{"X6", 6}})).build());

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

  auto
  eppstein_graph() -> Graph_type
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

  auto
  test_graph() -> Test_Graph<basic_type>
  {
    auto const built_graph = eppstein_graph();

    Test_Graph<basic_type> res;
    auto                  &nodes        = res.nodes;
    auto                  &dependencies = res.dependencies;
    auto                  &weights      = res.map_weight;

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
