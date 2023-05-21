//
// Created by faccus on 21/05/23.
//

#include "Graph_traits.h"
#include "KFinder_Factory.h"

#include "TestClass.h"
#include "TestGraph.h"
#include "chrono.h"

#include <fstream>
#include <gtest/gtest.h>

namespace network_butcher::kfinder
{
  template <bool Parallel_Edges, bool Reversed>
  class Weighted_Graph<
    network_butcher::types::WGraph<Parallel_Edges, network_butcher::types::Node, unsigned long long int>,
    Reversed,
    network_butcher::types::Node,
    std::vector<network_butcher::types::Node>,
    unsigned long long int> : base_Weighted_Graph
  {
  public:
    using Weight_Type = unsigned long long int;

    using Edge_Type = std::pair<node_id_type, node_id_type>;

    using Graph_Type =
      network_butcher::types::WGraph<Parallel_Edges, network_butcher::types::Node, unsigned long long int>;
    using Weight_Edge_Type = std::multiset<Weight_Type>;

    using Node_Type            = typename Graph_Type::Node_Type;
    using Node_Collection_Type = std::vector<Node_Type>;


    [[nodiscard]] Weight_Edge_Type
    get_weight(Edge_Type const &edge) const
    {
      if constexpr (Reversed && Parallel_Edges)
        {
          return graph.get_weight(std::make_pair(edge.second, edge.first));
        }
      else if constexpr (Reversed)
        {
          return {graph.get_weight(std::make_pair(edge.second, edge.first))};
        }
      else if constexpr (Parallel_Edges)
        {
          return graph.get_weight(edge);
        }
      else
        {
          return {graph.get_weight(edge)};
        }
    }

    [[nodiscard]] std::size_t
    size() const
    {
      return graph.size();
    };

    [[nodiscard]] bool
    empty() const
    {
      return graph.empty();
    };

    [[nodiscard]] std::set<node_id_type> const &
    get_output_nodes(node_id_type const &id) const
    {
      if constexpr (Reversed)
        {
          return graph.get_input_nodes(id);
        }
      else
        {
          return graph.get_output_nodes(id);
        }
    };


    Node_Type const &
    operator[](node_id_type const &id) const
    {
      return graph[id];
    };

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    cbegin() const
    {
      return graph.cbegin();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    cend() const
    {
      return graph.cend();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    begin() const
    {
      return cbegin();
    }

    [[nodiscard]] typename Node_Collection_Type::const_iterator
    end() const
    {
      return cend();
    }


    Weighted_Graph<Graph_Type, !Reversed, Node_Type, Node_Collection_Type, Weight_Type>
    reverse() const
    {
      return Weighted_Graph<Graph_Type, !Reversed, Node_Type, Node_Collection_Type, Weight_Type>(graph);
    }


    explicit Weighted_Graph(Graph_Type const &g)
      : base_Weighted_Graph()
      , graph(g)
    {}

    ~Weighted_Graph() override = default;

  private:
    Graph_Type const &graph;

    void
    fake_method() override{};
  };
} // namespace network_butcher::kfinder

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


  TEST(KFinderSuite, ParallelEdgesEppsteinTest)
  {
    std::vector<std::string> files{"almost_path_00",     "almost_path_01",     "almost_path_02",     "dense_00",
                                   "dense_01",           "example_00",         "max_random_00",      "max_random_01",
                                   "max_random_02",      "max_random_03",      "max_random_04",      "path_00",
                                   "random_00",          "random_01",          "random_02",          "random_03",
                                   "random_04",          "small_random_00",    "small_random_01",    "small_random_02",
                                   "smallest_random_00", "smallest_random_01", "smallest_random_02", "sparse_00",
                                   "sparse_01"};

    auto &factory = KFinder_Factory<Graph_type_Parallel, true, Weighted_Graph_Parallel_type<false>>::Instance();
    for (auto const &file_name : files)
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
    std::vector<std::string> files{"almost_path_00",     "almost_path_01",     "almost_path_02",     "dense_00",
                                   "dense_01",           "example_00",         "max_random_00",      "max_random_01",
                                   "max_random_02",      "max_random_03",      "max_random_04",      "path_00",
                                   "random_00",          "random_01",          "random_02",          "random_03",
                                   "random_04",          "small_random_00",    "small_random_01",    "small_random_02",
                                   "smallest_random_00", "smallest_random_01", "smallest_random_02", "sparse_00",
                                   "sparse_01"};

    auto &factory = KFinder_Factory<Graph_type_Parallel, true, Weighted_Graph_Parallel_type<false>>::Instance();
    for (auto const &file_name : files)
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

} // namespace