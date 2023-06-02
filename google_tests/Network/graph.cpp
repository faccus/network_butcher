//
// Created by faccus on 11/09/21.
//

#include "graph.h"
#include "test_class.h"
#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;

  using basic_type    = int;
  using Input         = Test_Class<basic_type>;
  using Content_type  = Content<Input>;
  using Graph_type    = Graph<CNode<Content_type>>;
  using IO_collection = io_collection_type<Input>;
  using Node_type     = Graph_type::Node_Type;

  TEST(GraphTests, DefaultConstructors)
  {
    Graph<CNode<Input>>     graph_empty;
    Graph<graph_input_type> graph_empty2;
  }

  TEST(GraphTests, ConstructorFromCustomClass)
  {
    int number_of_nodes = 10;

    std::vector<Node_type> nodes;

    nodes.emplace_back(std::move(Content_Builder<Input>().set_output({{"X", 0}})).build());

    for (int i = 1; i < number_of_nodes - 1; ++i)
      {
        nodes.emplace_back(
          std::move(Content_Builder<Input>().set_input({{"X", (i - 1) * 10}}).set_output({{"X", i * 10}})).build());
      }
    nodes.emplace_back(std::move(Content_Builder<Input>().set_input({{"X", (number_of_nodes - 2) * 10}})).build());

    Graph_type graph(nodes);
  }

} // namespace