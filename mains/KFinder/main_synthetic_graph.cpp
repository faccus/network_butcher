#include <iostream>
#include <random>

#include "GetPot"
#include "chrono.h"

#include "network_butcher.h"


/*
 * In this file, we test how long does the Eppstein and Lazy Eppstein algorithm take to compute the K shortest paths on
 * a "synthetic" graph, that is an emulation of the block graph. We, in particular, test how long does the algortihm take
 * to generate K paths both in the only distance and path case, and both for Eppstein and Lazy Eppstein given the number
 * of nodes
 * */

using namespace network_butcher;
using namespace network_butcher::types;

using Node_type = Node;
using GraphType = WGraph<false, Node_type>;


template <class Graph>
void
basic_weight(Graph &graph, bool fully_random = false)
{
  std::size_t seed;

  if (fully_random)
    {
      std::random_device rd;
      seed = rd();
    }
  else
    {
      seed = 0;
    }
  std::default_random_engine random_engine{seed};

  std::uniform_real_distribution<GraphType::Weight_Type> node_weights_generator(0., 100.);

  for (std::size_t tail = 0; tail < graph.get_nodes().size(); ++tail)
    for (auto const &head : graph.get_output_nodes(tail))
      {
        auto const tmp_weight = node_weights_generator(random_engine);
        graph.set_weight(std::make_pair(tail, head), tmp_weight);
      }
}

GraphType
basic_graph(std::size_t base_nodes, std::size_t num_devices = 3)
{
  if (base_nodes < 2)
    {
      throw std::runtime_error("Number of nodes must be at least 2");
    }

  GraphType::Dependencies_Type deps(2 + (base_nodes - 2) * num_devices);


  for (std::size_t i = 1; i < 1 + num_devices; ++i)
    {
      deps[0].second.insert(i);
      deps[i].first.insert(0);
    }

  for (std::size_t i = 2; i < base_nodes - 1; ++i)
    {
      for (std::size_t k = 1; k <= num_devices; ++k)
        {
          for (std::size_t j = 1; j <= num_devices; ++j)
            {
              deps[(i - 1) * num_devices + k].first.insert((i - 2) * num_devices + j);
              deps[(i - 2) * num_devices + j].second.insert((i - 1) * num_devices + k);
            }
        }
    }

  for (std::size_t k = 1; k < 1 + num_devices; ++k)
    {
      deps[(base_nodes - 2) * num_devices + 1].first.insert((base_nodes - 3) * num_devices + k);
      deps[(base_nodes - 3) * num_devices + k].second.insert((base_nodes - 2) * num_devices + 1);
    }

  return GraphType(std::vector<Node_type>(2 + (base_nodes - 2) * num_devices), std::move(deps));
};

std::size_t
simple_pow(std::size_t base, std::size_t exp)
{
  std::size_t res = 1;
  for (; exp > 0; --exp)
    {
      res *= base;
    }

  return res;
}

int
main(int argc, char **argv)
{
  GetPot      command_line(argc, argv);
  std::string export_path = "report_Synthetic_Graph.txt";

  std::vector<std::tuple<std::string, Time_Type>> results;
  std::size_t                                     num_tests       = command_line("num_tests", 10);
  std::size_t                                     max_power_nodes = command_line("max_power_nodes", 17);
  std::size_t                                     max_power_K     = command_line("max_power_K", 13);
  std::size_t                                     num_devices     = command_line("num_devices", 3);

#if PARALLEL_OPENMP
  std::cout << "Is OpenMP enabled? Let's check it!" << std::endl;

  int nthreads, tid;

#  pragma omp parallel default(none) private(nthreads, tid)
  {
    /* Obtain thread number */
    tid = omp_get_thread_num();
    printf("Hello world from omp thread %d\n", tid);

    /* Only master thread does this */
    if (tid == 0)
      {
        nthreads = omp_get_num_threads();
        printf("Number of threads = %d\n", nthreads);
      }

  } /* All threads join master thread and disband */
#endif


  Chrono crono;

  {
    std::ofstream out_file(export_path);
    out_file << "NumNodes,K,EppDist,EppPath,LazyDist,LazyPath" << std::endl;
    out_file.close();
  }

  for (std::size_t power_nodes = 5; power_nodes <= max_power_nodes; ++power_nodes)
    {
      std::size_t nodes = simple_pow(2, power_nodes);

      auto graph = basic_graph(nodes, num_devices);
      basic_weight(graph);

      std::cout << "Nodes: " << graph.size() << std::endl << std::endl;

      for (std::size_t power_k = 6; power_k <= max_power_K; ++power_k)
        {
          std::size_t K = simple_pow(2, power_k);

          std::cout << "K: " << K << std::endl;

          Time_Type time_eppstein_dist = 0., time_lazy_dist = 0.;
          Time_Type time_eppstein_path = 0., time_lazy_path = 0.;

          bool end_prematurely = false;

          for (std::size_t test_num = 0; test_num < num_tests; ++test_num)
            {
              Time_Type local_eppstein_dist = 0., local_lazy_dist = 0.;
              Time_Type local_eppstein_path = 0., local_lazy_path = 0.;

              {
                crono.start();
                auto const res = network_butcher::kfinder::KFinder_Factory<GraphType, false>::Instance()
                                   .create("eppstein", graph, 0, graph.get_nodes().back().get_id())
                                   ->compute(K);
                crono.stop();

                if (res.size() < K)
                  {
                    end_prematurely = true;
                    break;
                  }

                Time_Type local_time = crono.wallTime();
                local_eppstein_path += local_time;
                time_eppstein_path += local_time;
              }

              {
                crono.start();
                auto const res = network_butcher::kfinder::KFinder_Factory<GraphType, true>::Instance()
                                   .create("eppstein", graph, 0, graph.get_nodes().back().get_id())
                                   ->compute(K);
                crono.stop();

                if (res.size() < K)
                  {
                    throw std::runtime_error(
                      "Eppstein Dist failed: the number of paths does not match the expected one!");
                  }

                Time_Type local_time = crono.wallTime();
                local_eppstein_dist += local_time;
                time_eppstein_dist += local_time;
              }

              {
                crono.start();
                auto const res = network_butcher::kfinder::KFinder_Factory<GraphType, false>::Instance()
                                   .create("lazy_eppstein", graph, 0, graph.get_nodes().back().get_id())
                                   ->compute(K);
                crono.stop();

                if (res.size() < K)
                  {
                    throw std::runtime_error(
                      "Lazy Eppstein Path failed: the number of paths does not match the expected one!");
                  }

                Time_Type local_time = crono.wallTime();
                local_lazy_path += local_time;
                time_lazy_path += local_time;
              }

              {
                crono.start();
                auto const res = network_butcher::kfinder::KFinder_Factory<GraphType, true>::Instance()
                                   .create("lazy_eppstein", graph, 0, graph.get_nodes().back().get_id())
                                   ->compute(K);
                crono.stop();

                if (res.size() < K)
                  {
                    throw std::runtime_error(
                      "Lazy Eppstein Dist failed: the number of paths does not match the expected one!");
                  }

                Time_Type local_time = crono.wallTime();
                local_lazy_dist += local_time;
                time_lazy_dist += local_time;
              }


              std::cout << "Test #" << Utilities::custom_to_string(test_num + 1) << ": EppDist "
                        << local_eppstein_dist / 1000. << " ms, EppPath " << local_eppstein_path / 1000.
                        << " ms, LazyDist " << local_lazy_dist / 1000. << " ms, LazyPath " << local_lazy_path / 1000.
                        << " ms" << std::endl;
            }

          if (end_prematurely)
            {
              std::cout << "Prematurely ended! Skipping to next graph" << std::endl;
              break;
            }


          time_eppstein_dist /= (num_tests * static_cast<long double>(1000.));
          time_eppstein_path /= (num_tests * static_cast<long double>(1000.));

          time_lazy_dist /= (num_tests * static_cast<long double>(1000.));
          time_lazy_path /= (num_tests * static_cast<long double>(1000.));

          std::cout << std::endl
                    << "Total time average for " << graph.size() << " nodes and K: " << K << " is: "
                    << "EppDist " << time_eppstein_dist << " ms, EppPath " << time_eppstein_path << " ms, LazyDist "
                    << time_lazy_dist << " ms, LazyPath " << time_lazy_path << " ms" << std::endl
                    << std::endl;

          std::ofstream out_file(export_path, std::ios_base::app);
          out_file << graph.size() << "," << K << "," << time_eppstein_dist << "," << time_eppstein_path << ","
                   << time_lazy_dist << "," << time_lazy_path << std::endl;
          out_file.close();
        }
    }
}