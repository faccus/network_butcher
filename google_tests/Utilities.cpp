//
// Created by faccus on 30/08/21.
//
#include "../include/Utilities.h"
#include <gtest/gtest.h>

namespace {
  TEST(UtilitiesTestSuit, ComputeMemoryUsageFromEnumTest)
  {
    auto res = network_butcher_utilities::compute_memory_usage_from_enum(1);
    ASSERT_EQ(res, sizeof(float));

    res = network_butcher_utilities::compute_memory_usage_from_enum(-1);
    ASSERT_EQ(res, 0);
  }

  TEST(UtilitiesTestSuit, ParseOnnxFileTest)
  {
    const std::string model_path = "resnet18-v2-7-inferred.onnx";
    auto const        res        = network_butcher_utilities::parse_onnx_file(model_path);
  }

  TEST(UtilitiesTestSuit, OutputOnnxFileTest)
  {
    std::string model_path      = "resnet18-v2-7-inferred.onnx";
    std::string model_path_copy = "resnet18-v2-7-inferred-copy.onnx";

    if (network_butcher_utilities::file_exists(model_path_copy))
      std::filesystem::remove(model_path_copy);

    network_butcher_utilities::output_onnx_file(network_butcher_utilities::parse_onnx_file(model_path), model_path_copy);

    EXPECT_TRUE(network_butcher_utilities::file_exists(model_path_copy));
    std::filesystem::remove(model_path_copy);
  }
}