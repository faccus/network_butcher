//
// Created by faccus on 7/22/22.
//

#ifndef NETWORK_BUTCHER_DYNAMICTYPE_H
#define NETWORK_BUTCHER_DYNAMICTYPE_H

#include <string>
#include <variant>
#include <vector>

namespace network_butcher
{

  namespace types
  {
    using DynamicType = std::variant<std::vector<long int>, std::vector<float>, std::vector<std::string>>;
  } // namespace types

} // namespace network_butcher

#endif // NETWORK_BUTCHER_DYNAMICTYPE_H
