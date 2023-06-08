//
// Created by faccus on 30/05/23.
//
#include "graph_traits.h"
#include "kfinder_factory.h"
#include "weighted_graph_specialization.h"

#include "chrono.h"
#include "test_class.h"
#include "test_graph.h"

#include "GetPot"

#include <fstream>

using namespace network_butcher;
using namespace types;
using namespace network_butcher::kfinder;

using Node_type           = types::Node;
using Test_Weight_Type    = unsigned long long int;
using Graph_type_Parallel = types::WGraph<true, Node_type, Test_Weight_Type>;

template <bool Reversed>
using Weighted_Graph_Parallel_type = Weighted_Graph<Graph_type_Parallel,
                                                    Reversed,
                                                    Graph_type_Parallel::Node_Type,
                                                    Graph_type_Parallel::Node_Collection_Type,
                                                    Test_Weight_Type>;


std::tuple<Graph_type_Parallel, Node_Id_Type, Node_Id_Type, Node_Id_Type>
import_graph(std::string file_path)
{
  std::size_t                N, M, s, t, k, u, v;
  network_butcher::Time_Type tmp_weight;

  std::ifstream in_file(file_path);
  in_file >> N >> M >> s >> t >> k;

  std::vector<network_butcher::types::Node>    nodes(N);
  Converted_Onnx_Graph_Type::Dependencies_Type deps(N);
  std::vector<std::pair<std::pair<network_butcher::Node_Id_Type, network_butcher::Node_Id_Type>, Test_Weight_Type>>
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

// The initial files were generated by: https://github.com/yosupo06/library-checker-problems
std::vector<std::string>
get_test_names()
{
  return {"almost_path_00",
          "almost_path_01",
          "almost_path_02",
          "dense_00",
          "dense_01",
          "example_00",
          "loop_00",
          "max_random_00",
          "max_random_01",
          "max_random_02",
          "max_random_03",
          "max_random_04",
          "path_00",
          "random_00",
          "random_01",
          "random_02",
          "random_04",
          "small_random_00",
          "small_random_01",
          "small_random_02",
          "smallest_random_00",
          "smallest_random_01",
          "smallest_random_02",
          "sparse_00",
          "sparse_01"};
}


int
main(int argc, char **argv)
{
  GetPot command_line(argc, argv);

  std::size_t num_tests = command_line("num_tests", 10);

  Chrono crono;

  std::vector<std::tuple<std::string, long double, long double>> results;
  auto &factory = KFinder_Factory<Graph_type_Parallel, true, Weighted_Graph_Parallel_type<false>>::Instance();
  for (auto const &file_name : get_test_names())
    {
      std::string input = "test_data/kfinder/in/" + file_name + ".in";

      std::cout << "Processing file: " << input << std::endl;

      auto [graph, root, sink, k] = import_graph(input);
      long double epp = 0., lazy = 0.;

      {
        // Initial call to initialize all the factory related stuff
        factory.create("eppstein", graph, root, sink);
        factory.create("lazy_eppstein", graph, root, sink);
      }

      std::cout << "Eppstein: " << std::endl;
      for (std::size_t i = 0; i < num_tests; ++i)
        {
          crono.start();
          auto const res = factory.create("eppstein", graph, root, sink)->compute(k);
          crono.stop();
          long double time = crono.wallTime();

          std::cout << "Test #" << Utilities::custom_to_string(i + 1) << " took " << time / 1000. << " ms" << std::endl;

          epp += time;
        }

      std::cout << std::endl << "Lazy Eppstein: " << std::endl;
      for (std::size_t i = 0; i < num_tests; ++i)
        {
          crono.start();
          auto const res = factory.create("lazy_eppstein", graph, root, sink)->compute(k);
          crono.stop();
          long double time = crono.wallTime();

          std::cout << "Test #" << Utilities::custom_to_string(i + 1) << " took " << time / 1000. << " ms" << std::endl;

          lazy += time;
        }

      std::cout << std::endl
                << "Eppstein " << epp / (num_tests * static_cast<long double>(1000.)) << " ms, Lazy Eppstein "
                << lazy / (num_tests * static_cast<long double>(1000.)) << " ms" << std::endl
                << std::endl;

      results.emplace_back(file_name,
                           epp / (num_tests * static_cast<long double>(1000.)),
                           lazy / (num_tests * static_cast<long double>(1000.)));
    }

  std::string   export_path = "report_kfinder.txt";
  std::ofstream out_file(export_path);

  out_file << "Test,Eppstein,LazyEppstein" << std::endl;
  for (auto const &[name, epp, lazy] : results)
    out_file << name << "," << epp << "," << lazy << std::endl;
}