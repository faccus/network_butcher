//
// Created by faccus on 7/22/22.
//

#ifndef NETWORK_BUTCHER_DYNAMICTYPE_H
#define NETWORK_BUTCHER_DYNAMICTYPE_H

#include <string>
#include <vector>

namespace network_butcher_types
{
  /// @brief A simple "helper" type used to store either an int, a float, a string or a collection of the previous
  class DynamicType
  {
  private:
    std::vector<long int>    ints;
    std::vector<float>       floats;
    std::vector<std::string> strings;

    bool assigned;

  public:
    DynamicType()
      : ints{}
      , floats{}
      , strings{}
      , assigned{false} {};

    DynamicType(long int i)
      : ints{i}
      , floats{}
      , strings{}
      , assigned{true} {};
    DynamicType(std::vector<long int> const &_ints)
      : ints{_ints}
      , floats{}
      , strings{}
      , assigned{true} {};

    DynamicType(float i)
      : ints{}
      , floats{i}
      , strings{}
      , assigned{true} {};
    DynamicType(std::vector<float> const &_floats)
      : ints{}
      , floats{_floats}
      , strings{}
      , assigned{true} {};

    DynamicType(std::string str)
      : ints{}
      , floats{}
      , strings{str}
      , assigned{true} {};
    DynamicType(std::vector<std::string> const &strs)
      : ints{}
      , floats{}
      , strings{strs}
      , assigned{true} {};


    inline bool
    is_assigned() const
    {
      return assigned;
    }


    inline bool
    has_ints() const
    {
      return is_assigned() && ints.size() >= 1;
    }
    inline bool
    has_int() const
    {
      return is_assigned() && ints.size() == 1;
    }

    inline std::vector<long int> const &
    get_ints() const
    {
      return ints;
    }
    inline long int const &
    get_int() const
    {
      return ints.front();
    }


    inline bool
    has_floats() const
    {
      return is_assigned() && floats.size() >= 1;
    }
    inline bool
    has_float() const
    {
      return is_assigned() && floats.size() == 1;
    }

    inline std::vector<float> const &
    get_floats() const
    {
      return floats;
    }
    inline float const &
    get_float() const
    {
      return floats.front();
    }


    inline bool
    has_strings() const
    {
      return is_assigned() && strings.size() >= 1;
    }
    inline bool
    has_string() const
    {
      return is_assigned() && strings.size() == 1;
    }

    inline std::vector<std::string> const &
    get_strings() const
    {
      return strings;
    }
    inline std::string const &
    get_string() const
    {
      return strings.front();
    }
  };
} // namespace network_butcher_types

#endif // NETWORK_BUTCHER_DYNAMICTYPE_H
