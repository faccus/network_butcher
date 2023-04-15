//
// Created by faccus on 7/22/22.

//
#include <gtest/gtest.h>
#include <iostream>

#include "General_Manager.h"


namespace
{
  using namespace network_butcher;
  using namespace network_butcher::io;

  void
  execute_boot(std::string const &export_path, std::string const &conf_path)
  {
    if (Utilities::directory_exists(export_path))
      {
        Utilities::directory_delete(export_path);
      }

    EXPECT_NO_THROW(General_Manager::boot(conf_path, true));
  }

  TEST(GeneralManagerTest, boot_test)
  {
    execute_boot("ksp_result", "test_data/configs/test_parameters.conf");
  }

  TEST(GeneralManagerTest, boot_test2)
  {
    execute_boot("ksp_result2", "test_data/configs/test2_parameters.conf");
  }

  TEST(GeneralManagerTest, boot_test3)
  {
    execute_boot("ksp_result3", "test_data/configs/test3_parameters.conf");
  }

  TEST(GeneralManagerTest, boot_test4)
  {
    execute_boot("ksp_result4", "test_data/configs/test4_parameters.conf");
  }

  TEST(GeneralManagerTest, boot_test5)
  {
    execute_boot("ksp_result5", "test_data/configs/test5_parameters.conf");
  }
} // namespace
