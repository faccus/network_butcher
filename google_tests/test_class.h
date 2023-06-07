#ifndef NETWORK_BUTCHER_TEST_CLASS_H
#define NETWORK_BUTCHER_TEST_CLASS_H

#include "basic_traits.h"

template <class T>
class Test_Class
{
public:
  std::vector<T> data;
  Test_Class(int n)
    : data(n){};

  Test_Class()                        = default;
  Test_Class(const Test_Class &) = default;
  Test_Class(Test_Class &&)      = default;
  Test_Class &
  operator=(const Test_Class &) = default;

  network_butcher::Memory_Type
  compute_memory_usage() const
  {
    return data.size() * sizeof(T);
  }
};

#endif // NETWORK_BUTCHER_TEST_CLASS_H
