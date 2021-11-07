//
// Created by faccus on 07/11/21.
//

#include "Computer_time.h"


time_type
Computer_time::compute_time_usage(const Graph<graph_input_type> &graph,
                                  const Node                    &node)
{
  time_type res = .0;


  return res;
}


time_type
Computer_time::compute_time_usage(const Graph<graph_input_type> &graph,
                                  node_id_type                   id)
{
  return compute_time_usage(graph, graph.nodes[id]);
}
