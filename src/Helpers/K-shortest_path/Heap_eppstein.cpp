//
// Created by faccus on 03/11/21.
//

#include "Heap_eppstein.h"


bool
operator<(std::shared_ptr<H_out> const &lhs, std::shared_ptr<H_out> const &rhs)
{
  return *lhs < *rhs;
};