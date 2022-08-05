//
// Created by faccus on 07/11/21.
//
#include "../../include/Computer/Hardware_specifications.h"

network_butcher_computer::Hardware_specifications::Hardware_specifications(hardware_id_type in_id)
  : id(in_id)
{}

const std::string &
network_butcher_computer::Hardware_specifications::getName() const
{
  return id;
}


std::pair<time_type, time_type>
network_butcher_computer::Hardware_specifications::get_regression_coefficients(operation_id_type operation_id) const
{
  auto const it = regression_coefficients.find(operation_id);

  if (it != regression_coefficients.cend())
    return it->second;
  return std::pair<time_type, time_type>(-1., -1.);
}


void
network_butcher_computer::Hardware_specifications::set_regression_coefficient(operation_id_type               op_id,
                                                                              std::pair<time_type, time_type> coeff)
{
  regression_coefficients[op_id] = coeff;
}
