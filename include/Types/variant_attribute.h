//
// Created by faccus on 7/22/22.
//

#ifndef NETWORK_BUTCHER_VARIANT_ATTRIBUTE_H
#define NETWORK_BUTCHER_VARIANT_ATTRIBUTE_H

#include <string>
#include <variant>
#include <vector>

namespace network_butcher::types
{
  using Variant_Attribute = std::variant<std::vector<long int>, std::vector<float>, std::vector<std::string>>;
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_VARIANT_ATTRIBUTE_H
