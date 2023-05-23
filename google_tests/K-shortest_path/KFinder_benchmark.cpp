#include "Graph_traits.h"
#include "KFinder_Factory.h"
#include "KFinder_Test_Graph_Specialization.h"

#include "TestClass.h"
#include "TestGraph.h"
#include "chrono.h"

#include <fstream>
#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;
  using namespace types;
  using namespace network_butcher::kfinder;

  using basic_type  = int;
  using type_weight = double;

  using Input               = TestMemoryUsage<basic_type>;
  using Content_input       = types::Content<Input>;
  using Node_type           = types::Node;
  using Test_Weight_Type    = unsigned long long int;
  using Graph_type_Parallel = types::WGraph<true, Node_type, Test_Weight_Type>;

  template <bool Reversed>
  using Weighted_Graph_Parallel_type = Weighted_Graph<Graph_type_Parallel,
                                                      Reversed,
                                                      Graph_type_Parallel::Node_Type,
                                                      Graph_type_Parallel::Node_Collection_Type,
                                                      Test_Weight_Type>;

  std::tuple<Graph_type_Parallel, node_id_type, node_id_type, node_id_type>
  import_graph(std::string file_path);

  std::vector<std::string>
  get_test_names();


  TEST(KFinderSuite, ParallelEdgesEppsteinTest)
  {
    auto &factory = KFinder_Factory<Graph_type_Parallel, true, Weighted_Graph_Parallel_type<false>>::Instance();
    for (auto const &file_name : get_test_names())
      {
        std::string input = "test_data/kfinder/in/" + file_name + ".in";

        std::cout << "Testing file: " << input;

        auto [graph, root, sink, k] = import_graph(input);

        Chrono crono;
        crono.start();
        auto const res = factory.create("eppstein", graph, root, sink)->compute(k);
        crono.stop();

        std::string output = "test_data/kfinder/out/" + file_name + ".out";

        std::ifstream    result_file(output);
        std::size_t      current_size = 0;
        Test_Weight_Type val;

        for (; current_size < res.size(); ++current_size)
          {
            result_file >> val;
            ASSERT_EQ(val, res[current_size]);
          }

        if (res.size() < k)
          {
            double last = .0;
            result_file >> last;

            ASSERT_EQ(last, -1.);
          }

        std::cout << " SUCCESS, " << crono.wallTime() / 1000. << " ms" << std::endl;
      }
  }


  TEST(KFinderSuite, ParallelEdgesLazyEppsteinTest)
  {
    auto &factory = KFinder_Factory<Graph_type_Parallel, true, Weighted_Graph_Parallel_type<false>>::Instance();
    for (auto const &file_name : get_test_names())
      {
        std::string input = "test_data/kfinder/in/" + file_name + ".in";

        std::cout << "Testing file: " << input;

        auto [graph, root, sink, k] = import_graph(input);

        Chrono crono;
        crono.start();
        auto const res = factory.create("lazy_eppstein", graph, root, sink)->compute(k);
        crono.stop();

        std::string output = "test_data/kfinder/out/" + file_name + ".out";

        std::ifstream    result_file(output);
        std::size_t      current_size = 0;
        Test_Weight_Type val;

        for (; current_size < res.size(); ++current_size)
          {
            result_file >> val;
            ASSERT_EQ(val, res[current_size]);
          }

        if (res.size() < k)
          {
            double last = .0;
            result_file >> last;

            ASSERT_EQ(last, -1.);
          }

        std::cout << " SUCCESS, " << crono.wallTime() / 1000. << " ms" << std::endl;
      }
  }


  std::tuple<Graph_type_Parallel, node_id_type, node_id_type, node_id_type>
  import_graph(std::string file_path)
  {
    std::size_t                  N, M, s, t, k, u, v;
    network_butcher::weight_type tmp_weight;

    std::ifstream in_file(file_path);
    in_file >> N >> M >> s >> t >> k;

    std::vector<network_butcher::types::Node> nodes(N);
    graph_type::Dependencies_Type             deps(N);
    std::vector<
      std::pair<std::pair<network_butcher::node_id_type, network_butcher::node_id_type>, network_butcher::weight_type>>
      edges;
    edges.reserve(M);

    for (std::size_t i = 0; i < M; ++i)
      {
        in_file >> u >> v >> tmp_weight;
        edges.push_back({{u, v}, tmp_weight});

        deps[v].first.insert(u);
        deps[u].second.insert(v);
      }

    Graph_type_Parallel graph(nodes, deps);
    for (auto const &[edge, weight] : edges)
      graph.set_weight(edge, weight);

    return std::tie(graph, s, t, k);
  }

  std::vector<std::string>
  get_test_names()
  {
    return {"almost_path_00",
            "almost_path_01",
            "almost_path_02",
            "dense_00",
            "dense_01",
            "example_00",
            "path_00",
            "small_random_00",
            "small_random_02",
            "smallest_random_00",
            "smallest_random_01"};
  }

} // namespace