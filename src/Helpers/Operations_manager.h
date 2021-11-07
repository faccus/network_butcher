//
// Created by faccus on 07/11/21.
//

#ifndef NETWORK_BUTCHER_OPERATIONS_MANAGER_H
#define NETWORK_BUTCHER_OPERATIONS_MANAGER_H

#include "Traits/Hardware_traits.h"
#include <string>
#include <utility>

class Operations_manager
{
  std::string name;

public:
  explicit Operations_manager(std::string n)
    : name{std::move(n)} {};
};


#endif // NETWORK_BUTCHER_OPERATIONS_MANAGER_H
