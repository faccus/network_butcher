//
// Created by faccus on 30/08/21.
//
#include "Utilities.h"
#include <gtest/gtest.h>

namespace {
  using namespace network_butcher;

  TEST(UtilitiesTestSuit, VerifyProtobufVersionTest) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
  }

  TEST(UtilitiesTestSuit, ComputeMemoryUsageFromEnumTest)
  {
    auto res = Utilities::compute_memory_usage_from_enum(1);
    ASSERT_EQ(res, sizeof(float));

    res = Utilities::compute_memory_usage_from_enum(-1);
    ASSERT_EQ(res, 0);
  }

  TEST(UtilitiesTestSuit, ParseOnnxFileTest)
  {
    const std::string model_path = "resnet18-v2-7-inferred.onnx";
    auto const        res        = Utilities::parse_onnx_file(model_path);
  }

  TEST(UtilitiesTestSuit, OutputOnnxFileTest)
  {
    std::string model_path      = "resnet18-v2-7-inferred.onnx";
    std::string model_path_copy = "resnet18-v2-7-inferred-copy.onnx";

    if (Utilities::file_exists(model_path_copy))
      std::filesystem::remove(model_path_copy);

    Utilities::output_onnx_file(Utilities::parse_onnx_file(model_path),
                                                 model_path_copy);

    EXPECT_TRUE(Utilities::file_exists(model_path_copy));
    std::filesystem::remove(model_path_copy);
  }
}