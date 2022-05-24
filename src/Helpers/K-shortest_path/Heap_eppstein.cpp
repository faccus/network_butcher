//
// Created by faccus on 03/11/21.
//

#include "../../../include/Helpers/K-shortest_path/Heap_eppstein.h"


bool
operator<(std::shared_ptr<H_out<edge_info>> const &lhs,
          std::shared_ptr<H_out<edge_info>> const &rhs)
{
  return *lhs < *rhs;
};