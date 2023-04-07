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

  TEST(GeneralManagerTest, boot_test)
  {
    std::string const export_path = "ksp_result";
    if (Utilities::directory_exists(export_path))
      {
        Utilities::directory_delete(export_path);
      }

    std::string const path = "test_data/configs/test_parameters.conf";
    General_Manager::boot(path, true);
  }

  TEST(GeneralManagerTest, boot_test2)
  {
    std::string const export_path = "ksp_result2";
    if (Utilities::directory_exists(export_path))
      {
        Utilities::directory_delete(export_path);
      }

    std::string const path = "test_data/configs/test2_parameters.conf";
    General_Manager::boot(path, true);
  }

  TEST(GeneralManagerTest, boot_test3)
  {
    std::string const export_path = "ksp_result3";
    if (Utilities::directory_exists(export_path))
      {
        Utilities::directory_delete(export_path);
      }

    std::string const path = "test_data/configs/test3_parameters.conf";
    General_Manager::boot(path, true);
  }

  TEST(GeneralManagerTest, boot_test4)
  {
    std::string const export_path = "ksp_result4";
    if (Utilities::directory_exists(export_path))
      {
        Utilities::directory_delete(export_path);
      }

    std::string const path = "test_data/configs/test4_parameters.conf";
    General_Manager::boot(path, true);
  }

  TEST(GeneralManagerTest, boot_test5)
  {
    std::string const export_path = "ksp_result5";
    if (Utilities::directory_exists(export_path))
      {
        Utilities::directory_delete(export_path);
      }

    std::string const path = "test_data/configs/test5_parameters.conf";
    General_Manager::boot(path, true);
  }
} // namespace
