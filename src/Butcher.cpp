//
// Created by faccus on 24/08/21.
//

#include "Butcher.h"


std::vector<slice_type>
Butcher::compute_partial_two_slice_memory_brute_force(
  memory_type memory_first_slice) const
{
  // Compute all the basic routes
  auto slices             = compute_basic_routes();
  auto nodes_memory_usage = graph.compute_nodes_memory_usage_input();

  auto tester = [&nodes_memory_usage,
                 &memory_first_slice](const slice_type &slice) {
    memory_type memory_usage = 0;
    for (auto &j : slice)
      memory_usage += nodes_memory_usage[j];
    return memory_usage < memory_first_slice;
  };

  return partition_checker(slices, tester);
}


std::vector<slice_type>
Butcher::compute_basic_routes() const
{
  std::vector<slice_type> basic_routes;
  basic_routes.reserve(graph.nodes.size());

  {
    slice_type tmp;
    tmp.insert(0);
    basic_routes.push_back(tmp);
  }

  for (int i = 1; i < graph.nodes.size(); ++i)
    {
      slice_type partial_res;
      partial_res.insert(i);

      const auto &input_nodes = graph.dependencies[i].first;

      // Construct the current basic_route the i (just add {i} to the basic
      // routes of the parents).
      for (auto &node : input_nodes)
        partial_res.insert(basic_routes[node].begin(),
                           basic_routes[node].end());
      basic_routes.push_back(partial_res);
    }

  return basic_routes;
}


std::vector<slice_type>
Butcher::compute_two_slice_brute_force() const
{
  std::vector<slice_type> res;
  res.reserve(graph.nodes.size());

  slice_type start{0};
  res.push_back(start);

  for (int i = 1; i < graph.nodes.size(); ++i)
    {
      auto &dependencies = graph.dependencies[i];
      auto &input        = dependencies.first;

      for (auto &part : res)
        {
          bool ok = true;
          for (auto &in : input)
            if (!part.contains(in))
              {
                ok = false;
                break;
              }
          if (ok)
            {
              auto copy = part;
              copy.insert(i);
              res.push_back(copy);
            }
        }
    }

  return res;
}


std::vector<slice_type>
Butcher::compute_two_slice_memory_brute_force(memory_type memory_first_slice) const
{
  auto slices             = compute_two_slice_brute_force();
  auto nodes_memory_usage = graph.compute_nodes_memory_usage_input();

  auto tester = [&nodes_memory_usage,
                 &memory_first_slice](const slice_type &slice) {
    memory_type memory_usage = 0;
    for (auto &j : slice)
      memory_usage += nodes_memory_usage[j];
    return memory_usage < memory_first_slice;
  };

  return partition_checker(slices, tester);
}


std::vector<slice_type>
Butcher::partition_checker(std::vector<slice_type>                &slices,
                           const std::function<bool(const slice_type &)>& tester)
{
  std::vector<slice_type> res;

  for (int i = 0; i < slices.size(); ++i)
    {
      if (tester(slices[i]))
        res.push_back(std::move(slices[i]));
      else
        {
          {
            slice_type tmp(std::move(slices[i]));
          }
        }
    }

  return res;
}