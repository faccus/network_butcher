//
// Created by faccus on 24/08/21.
//

#include "Butcher.h"

std::vector<Slice_type>
Butcher::compute_two_slice_brute_force() const
{
  std::vector<Slice_type> res;
  res.reserve(graph.nodes.size());

  {
    Slice_type tmp; tmp.insert(0);
    res.push_back(tmp);
  }

  for(int i = 1; i < graph.nodes.size(); ++i)
    {
      Slice_type partial_res; partial_res.insert(i);
      Slice_type input_nodes;
      auto & node_inputs = graph.nodes[i].get_input();
      for (auto &in : node_inputs)
        {
          auto p = graph.appearances.find(in->get_name());
          if (p != graph.appearances.end())
            {
              auto &tmp_input_nodes = (*p).second;
              input_nodes.insert(tmp_input_nodes.begin(),
                                 tmp_input_nodes.end());
            }
        }

      for (auto &node : input_nodes)
        if (node < i)
          partial_res.insert(res[node].begin(), res[node].end());
      res.push_back(partial_res);
    }

  return res;
}

std::vector<Slice_type>
Butcher::compute_partial_two_slice_memory_brute_force(
  size_t memory_first_slice) const
{
  auto slices = compute_two_slice_brute_force();
  std::vector<Slice_type> res;
  auto nodes_memory_usage = graph.compute_nodes_memory_usage_input();

  for(int i = 0; i < slices.size(); ++i)
    {
      size_t memory_usage = 0;
      for(auto & j : slices[i])
        memory_usage += nodes_memory_usage[j];

      if(memory_usage < memory_first_slice)
        res.push_back(std::move(slices[i]));
    }

  return res;
}


/*
// Helper function:
// res is the current partial set of slices, index is the index of the current node, forward tells us if we are in a "middle" node of a branch
int
Butcher::partial_two_slice_brute_force_helper(std::vector<std::vector<int>> & res,
                                              int index, int level, int from_index, std::vector<int> stop_at, std::vector<int> rif) const
{
  if(index == graph.nodes.size()) // Finished
    return -1;

  bool end_junction =
    graph.nodes[index].get_input().size() > 1; // Here a juction ended
  bool start_junction =
    graph.nodes[index].get_output().size() > 1; // Here a junction started
  int net = graph.nodes[index].get_output().size() -
            graph.nodes[index].get_input().size();

  if(!end_junction && !start_junction)
    {
      std::vector<int> tmp(index + 1); std::iota(tmp.begin(), tmp.end(), 0);
      return partial_two_slice_brute_force_helper(res, index+1, level, from_index, stop_at, rif);
    }

  if(end_junction) {
      std::vector<int> tmp(index + 1); std::iota(tmp.begin(), tmp.end(), 0);
      return partial_two_slice_brute_force_helper(res, index+1, level, from_index, stop_at, rif);
    }

  if(start_junction) {
      if(level == 0)
        {
          {
            auto tmp = res.back();
            tmp.push_back(index);
            res.push_back(tmp);
          }

          std::set<int> output_nodes;
          for(auto & out : graph.nodes[index].get_output())
            {
              auto p = graph.appearances.find(out->get_name());
              if(p != graph.appearances.end())
                output_nodes.insert((*p).second.begin(), (*p).second.end());
            }

          std::vector<std::pair<int, int>> output;
          for (auto &node : output_nodes)
            output.push_back(std::make_pair(
              partial_two_slice_brute_force_helper(res, node, level + net),
              level + net));


        }
      else
        {

        }
    }

  if(start_junction) {

    }

  return -1;
}
*/