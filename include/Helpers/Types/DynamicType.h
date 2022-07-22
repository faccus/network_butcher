//
// Created by faccus on 7/22/22.
//

#ifndef NETWORK_BUTCHER_DYNAMICTYPE_H
#define NETWORK_BUTCHER_DYNAMICTYPE_H

#include <vector>
#include <string>

class DynamicType {
private:
  std::vector<int> ints;
  std::vector<std::size_t> longs;
  std::vector<float> floats;
  std::vector<std::string> strings;

  bool assigned;
public:
  DynamicType()
    : ints{}
    , floats{}
    , strings{}
    , assigned{false} {};

  DynamicType(int i)
    : ints{i}
    , longs{}
    , floats{}
    , strings{}
    , assigned{true} {};
  DynamicType(std::vector<int> const &_ints)
    : ints{_ints}
    , longs{}
    , floats{}
    , strings{}
    , assigned{true} {};

  DynamicType(std::size_t i)
    : ints{}
    , longs{i}
    , floats{}
    , strings{}
    , assigned{true} {};
  DynamicType(std::vector<std::size_t> const &_ints)
    : ints{}
    , longs{_ints}
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
    , longs{}
    , floats{_floats}
    , strings{}
    , assigned{true} {};

  DynamicType(std::string str)
    : ints{}
    , longs{}
    , floats{}
    , strings{str}
    , assigned{true} {};
  DynamicType(std::vector<std::string> const &strs)
    : ints{}
    , longs{}
    , floats{}
    , strings{strs}
    , assigned{true} {};


  inline bool is_assigned() const {
    return assigned;
  }


  inline bool has_ints() const {
    return is_assigned() && ints.size() >= 1;
  }
  inline bool has_int() const {
    return is_assigned() && ints.size() == 1;
  }

  inline std::vector<int> const &get_ints() const {
    return ints;
  }
  inline int const &get_int() const {
    return ints.front();
  }


  inline bool has_longs() const {
    return is_assigned() && longs.size() >= 1;
  }
  inline bool has_long() const {
    return is_assigned() && longs.size() == 1;
  }

  inline std::vector<std::size_t> const &get_longs() const {
    return longs;
  }
  inline std::size_t const &get_long() const {
    return longs.front();
  }


  inline bool has_floats() const {
    return is_assigned() && floats.size() >= 1;
  }
  inline bool has_float() const   {
    return is_assigned() && floats.size() == 1;
  }

  inline std::vector<float> const &get_floats() const {
    return floats;
  }
  inline float const &get_float() const {
    return floats.front();
  }


  inline bool has_strings() const {
    return is_assigned() && strings.size() >= 1;
  }
  inline bool has_string() const {
    return is_assigned() && strings.size() == 1;
  }

  inline std::vector<std::string> const &get_strings() const {
    return strings;
  }
  inline std::string const &get_string() const {
    return strings.front();
  }


};

#endif // NETWORK_BUTCHER_DYNAMICTYPE_H
