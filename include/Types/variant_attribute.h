#ifndef NETWORK_BUTCHER_VARIANT_ATTRIBUTE_H
#define NETWORK_BUTCHER_VARIANT_ATTRIBUTE_H

#include <string>
#include <variant>
#include <vector>

namespace network_butcher::types
{
  /// Variant_Attribute is a variant type that can hold a vector of long int, float or string.
  using Variant_Attribute = std::variant<std::vector<long int>, std::vector<float>, std::vector<std::string>>;
} // namespace network_butcher::types

#endif // NETWORK_BUTCHER_VARIANT_ATTRIBUTE_H
