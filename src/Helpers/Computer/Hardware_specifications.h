//
// Created by faccus on 14/10/21.
//

#ifndef NETWORK_BUTCHER_HARDWARE_SPECIFICATIONS_H
#define NETWORK_BUTCHER_HARDWARE_SPECIFICATIONS_H

#include "../Traits/Hardware_traits.h"
#include "../Traits/Node_traits.h"

#include <string>
#include <unordered_map>

class Hardware_specifications
{
  hardware_id_type id;


  std::unordered_map<operation_id_type, std::pair<time_type, time_type>>
    regression_coefficients;

public:
  explicit Hardware_specifications(hardware_id_type);


  [[nodiscard]] const std::string &
  getName() const;


  [[nodiscard]] std::pair<time_type, time_type>
    get_regression_coefficients(operation_id_type) const;


  void set_regression_coefficient(operation_id_type,
                                  std::pair<time_type, time_type>);
};

#endif // NETWORK_BUTCHER_HARDWARE_SPECIFICATIONS_H
