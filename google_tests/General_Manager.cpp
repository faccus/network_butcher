//
// Created by faccus on 7/22/22.

//
#include <gtest/gtest.h>
#include <iostream>

#include "../include/General_Manager.h"


namespace butcher_benchmark_test_namespace {
  TEST(GeneralManagerTest, boot_test)
  {
    std::string const path = "test_parameters.conf";
    General_Manager::boot(path);
  }

  TEST(GeneralManagerTest, boot_test2)
  {
    std::string const path = "test2_parameters.conf";
    General_Manager::boot(path);
  }
}
