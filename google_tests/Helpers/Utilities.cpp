//
// Created by faccus on 30/08/21.
//
#include <gtest/gtest.h>
#include "../../src/Helpers/Utilities.h"

TEST(UtilitiesTestSuit, ComputeMemoryUsageFromEnumTest) {
  auto res = utilities::compute_memory_usage_from_enum(1);
  ASSERT_EQ(res, sizeof(float));

  res = utilities::compute_memory_usage_from_enum(-1);
  ASSERT_EQ(res, -1);
}
