//
// Created by faccus on 24/08/21.
//

#include "Butcher.h"


std::vector<Slice_type>
Butcher::compute_partial_two_slice_memory_brute_force(
  size_t memory_first_slice) const
{
  // Compute all the basic routes
  auto slices             = compute_basic_routes();
  auto nodes_memory_usage = graph.compute_nodes_memory_usage_input();

  auto tester = [&nodes_memory_usage,
                 &memory_first_slice](const Slice_type &slice) {
    size_t memory_usage = 0;
    for (auto &j : slice)
      memory_usage += nodes_memory_usage[j];
    return memory_usage < memory_first_slice;
  };

  return partition_checker(slices, tester);
}


std::vector<Slice_type>
Butcher::compute_basic_routes() const
{
  std::vector<Slice_type> basic_routes;
  basic_routes.reserve(graph.nodes.size());

  {
    Slice_type tmp;
    tmp.insert(0);
    basic_routes.push_back(tmp);
  }

  for (int i = 1; i < graph.nodes.size(); ++i)
    {
      Slice_type partial_res;
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


std::vector<Slice_type>
Butcher::compute_two_slice_brute_force() const
{
  std::vector<Slice_type> res;
  res.reserve(graph.nodes.size());

  Slice_type start{0};
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


std::vector<Slice_type>
Butcher::compute_two_slice_memory_brute_force(size_t memory_first_slice) const
{
  auto slices             = compute_two_slice_brute_force();
  auto nodes_memory_usage = graph.compute_nodes_memory_usage_input();

  auto tester = [&nodes_memory_usage,
                 &memory_first_slice](const Slice_type &slice) {
    size_t memory_usage = 0;
    for (auto &j : slice)
      memory_usage += nodes_memory_usage[j];
    return memory_usage < memory_first_slice;
  };

  return partition_checker(slices, tester);
}


std::vector<Slice_type>
Butcher::partition_checker(std::vector<Slice_type>                &slices,
                           const std::function<bool(const Slice_type &)>& tester)
{
  std::vector<Slice_type> res;

  for (int i = 0; i < slices.size(); ++i)
    {
      if (tester(slices[i]))
        res.push_back(std::move(slices[i]));
      else
        {
          {
            Slice_type tmp(std::move(slices[i]));
          }
        }
    }

  return res;
}

/*


// WIP
int
Butcher::recursive_brute_forcer(std::set<Slice_type> &res,
                                const std::vector<Slice_type> &basic_routes,
                                Slice_type &slice,
                                int index,
                                int mode,
                                int end)
{
  auto & node = graph.nodes[index];

  const auto & input_nodes = graph.dependencies[index].first;
  const auto & output_nodes = graph.dependencies[index].second;

  if(mode == -1 && output_nodes.size() == 1) {
      // go deeper
      return recursive_brute_forcer(res, basic_routes, slice, index+1);
    }
  else if (mode == -1 && output_nodes.size() > 1)
    {
      // First branch, first time
      auto   basic_reference = basic_routes[index];
      size_t net             = output_nodes.size() - 1;
      size_t debt            = 0;
      int    end_junction    = 0;

      for (int end_junction = index + 1;
           end_junction < graph.nodes.size() && net == 0;
           ++end_junction)
        {
          if (debt > 0)
            {
              net += debt;
              debt = 0;
            }

          auto in   = graph.dependencies[end_junction].first.size();
          auto out  = graph.dependencies[end_junction].second.size();
          auto diff = out - in;

          if (in > 1 && out > 1)
            {
              debt = diff + out - 1;
              net -= (out-1);
            }
          else
            {
              net += diff;
            }
        }
      --end_junction;

      while( !basic_reference.contains(end_junction-1) ) {

        }


    }

  return 0;
}


// WIP
std::vector<Slice_type>
Butcher::partial_compute_two_slice_brute_force() const
{
  std::vector<Slice_type> basic_routes;
  basic_routes.reserve(graph.nodes.size());

  {
    Slice_type tmp; tmp.insert(0);
    basic_routes.push_back(tmp);
  }

  std::map<int, std::vector<std::vector<Slice_type>> >
                branch_partitions;
  std::set<int> junctions;

  for(int i = 1; i < graph.nodes.size(); ++i)
    {
      Slice_type partial_res;
      partial_res.insert(i);

      const auto & input_nodes  = graph.dependencies[i].first;
      const auto & output_nodes = graph.dependencies[i].second;

      // Construct the current basic_route the i (just add {i} to the basic routes of the parents).
      for (auto &node : input_nodes)
        partial_res.insert(basic_routes[node].begin(),
                           basic_routes[node].end());
      basic_routes.push_back(partial_res);

      // If here a junction ends, then:
      if (input_nodes.size() > 1)
        {
          auto &partition_in_branch = branch_partitions.rbegin()->second;

          // If we have multiple branches:
          if (partition_in_branch.size() > 1)
            {
              // Look at every basic route for the every branch
              for(int i = 0; i < partition_in_branch.size(); ++i) {
                  auto & partition = partition_in_branch[i];
                  // Look at every other branch...
                  for (int j = i+1; j < partition_in_branch.size(); ++j)
                    {
                      // and at their routes
                      for (auto &tmp : partition_in_branch[j])
                        {
                          // Finally, add them together
                        }
                    }
                }
            }
          if (branch_partitions.size() > 1)
            {}
        }
      // If it's a juction, add a new junction
      if(output_nodes.size() > 1) {
          branch_partitions[i];

          junctions.insert(i);
        }
      else if (branch_partitions.size() > 1 && output_nodes.size() == 1 &&
               input_nodes.size() == 1)
        {
          auto & partition_in_branch = branch_partitions.rbegin()->second;
          if(junctions.contains(*input_nodes.begin())) {
              partition_in_branch.push_back({});
            }

        }
    }

  return basic_routes;
}
 * */