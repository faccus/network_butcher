//
// Created by faccus on 15/10/21.
//

#ifndef NETWORK_BUTCHER_TESTCLASS_H
#define NETWORK_BUTCHER_TESTCLASS_H

#include "../src/Helpers/Traits/Basic_traits.h"

class TestMemoryUsage
{
public:
  std::vector<int> data;
  TestMemoryUsage(int n) : data(n) {};

  TestMemoryUsage() = default;
  TestMemoryUsage(const TestMemoryUsage&) = default;
  TestMemoryUsage(TestMemoryUsage&&) = default;
  TestMemoryUsage & operator = (const TestMemoryUsage&) = default;

  memory_type compute_memory_usage() const
  {
    return data.size() * sizeof(int);
  }


};



#endif // NETWORK_BUTCHER_TESTCLASS_H
