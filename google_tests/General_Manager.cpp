//
// Created by faccus on 7/22/22.

//
#include <gtest/gtest.h>
#include <iostream>

#include "../include/General_Manager.h"


namespace
{
  using namespace network_butcher_io;

  TEST(GeneralManagerTest, boot_test)
  {
    std::string const path = "test_parameters.conf";
    General_Manager::boot(path, true);
  }

  TEST(GeneralManagerTest, boot_test2)
  {
    std::string const path = "test2_parameters.conf";
    General_Manager::boot(path, true);
  }
} // namespace
