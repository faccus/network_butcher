//
// Created by faccus on 24/04/23.
//
#include "Extra_Constraint.h"

namespace network_butcher::constraints
{
  std::vector<std::unique_ptr<Extra_Constraint>>
  generate_constraints(parameters::Parameters const &params)
  {
    std::vector<std::unique_ptr<Extra_Constraint>> res;

    if (params.memory_constraint)
      {
        // res.push_back(std::make_unique<Memory_Constraint>());
      }

    return res;
  }
} // namespace network_butcher::constraints