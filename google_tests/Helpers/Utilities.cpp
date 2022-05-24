//
// Created by faccus on 30/08/21.
//
#include "../../include/Helpers/Utilities.h"
#include <gtest/gtest.h>

TEST(UtilitiesTestSuit, ComputeMemoryUsageFromEnumTest)
{
  auto res = utilities::compute_memory_usage_from_enum(1);
  ASSERT_EQ(res, sizeof(float));

  res = utilities::compute_memory_usage_from_enum(-1);
  ASSERT_EQ(res, -1);
}

TEST(UtilitiesTestSuit, ParseOnnxFileTest)
{
  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  auto const        res        = utilities::parse_onnx_file(model_path);
}

TEST(UtilitiesTestSuit, OutputOnnxFileTest)
{
  std::string model_path      = "resnet18-v2-7-inferred.onnx";
  std::string model_path_copy = "resnet18-v2-7-inferred-copy.onnx";

  if (utilities::file_exists(model_path_copy))
    std::filesystem::remove(model_path_copy);

  utilities::output_onnx_file(utilities::parse_onnx_file(model_path),
                              model_path_copy);

  EXPECT_TRUE(utilities::file_exists(model_path_copy));
  std::filesystem::remove(model_path_copy);
}