#include <gtest/gtest.h>
#include <iostream>

#include <network_butcher/general_manager.h>

/// Checks if the boot tests run properly. The results must be checked manually.

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

  // It should run properly (no message in cout) and produce 10 paths
  TEST(GeneralManagerTest, boot_test1)
  {
    execute_boot("ksp_result", "test_data/configs/test1_parameters.conf");
  }

  // It should run properly (no message in cout) and produce 10 paths
  TEST(GeneralManagerTest, boot_test2)
  {
    execute_boot("ksp_result2", "test_data/configs/test2_parameters.conf");
  }

  // It should run properly (no message in cout) and produce 10 paths
  TEST(GeneralManagerTest, boot_test3)
  {
    execute_boot("ksp_result3", "test_data/configs/test3_parameters.conf");
  }

#if NETWORK_BUTCHER_PYBIND_ACTIVE

  // It should run properly and produce 12 paths. Some non-suppressible messages from aMLLibrary will appear
  TEST(GeneralManagerTest, boot_test4)
  {
    execute_boot("ksp_result4", "test_data/configs/test4_parameters.conf");
  }
#endif

  // It should run with a warning from Memory_Constraint (that will not be enforced) and produce a single path
  TEST(GeneralManagerTest, boot_test5)
  {
    execute_boot("ksp_result5", "test_data/configs/test5_parameters.conf");
  }

  // It should run properly (no message in cout) and produce 50 paths
  TEST(GeneralManagerTest, boot_test7)
  {
    execute_boot("ksp_result7", "test_data/configs/test7_parameters.conf");
  }
} // namespace
