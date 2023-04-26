//
// Created by faccus on 24/04/23.
//

#ifndef NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H
#define NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H

#include "Graph_traits.h"
#include "Parameters.h"

namespace network_butcher::constraints
{
  class Extra_Constraint
  {
  public:
    Extra_Constraint() = default;

    virtual void
    apply_constraint(block_graph_type &graph) = 0;

    virtual ~Extra_Constraint() = default;
  };

  class Memory_Constraint : public Extra_Constraint
  {
  private:
    parameters::Memory_Constraint_Type mode;

  public:
    Memory_Constraint(parameters::Memory_Constraint_Type mode)
      : mode{mode} {};

    void
    apply_constraint(block_graph_type &graph) override;

    ~Memory_Constraint() = default;
  };

  std::vector<std::unique_ptr<Extra_Constraint>>
  generate_constraints(parameters::Parameters const &params);
} // namespace network_butcher::constraints

#endif // NETWORK_BUTCHER_EXTRA_CONDITION_BUILDER_H
