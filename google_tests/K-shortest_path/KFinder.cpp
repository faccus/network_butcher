//
// Created by faccus on 21/04/23.
//
#include "Graph_traits.h"
#include "KEppstein.h"
#include "KEppstein_lazy.h"

#include "TestClass.h"
#include "TestGraph.h"
#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher::kfinder;
  using test_graph_type = TestGraph<int>;

  TEST(KFinderTests, CheckKFinderTypeSimpleGraph)
  {
    auto const res = std::is_base_of_v<KFinder<test_graph_type>, decltype(KFinder_Eppstein(test_graph_type()))>;
    ASSERT_TRUE(res);
  }

  TEST(KFinderTests, CheckKFinderTypeWeightedGraph)
  {
    Weighted_Graph<test_graph_type> weighted_graph((test_graph_type()));

    auto const res = std::is_base_of_v<KFinder<TestGraph<int>>, decltype(KFinder_Eppstein(weighted_graph))>;
    ASSERT_TRUE(res);
  }
} // namespace