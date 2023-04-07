//
// Created by faccus on 30/08/21.
//
#include "Utilities.h"
#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;
  std::string const base_path_no_extension = "test_data/models/resnet18-v2-7-inferred";

  TEST(UtilitiesTestSuit, VerifyProtobufVersionTest)
  {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
  }

  TEST(UtilitiesTestSUit, ParseNonExistingOnnxFileTest)
  {
    EXPECT_THROW(Utilities::parse_onnx_file("this_model_doesnt_exists.onnx"), std::runtime_error);
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
    EXPECT_NO_THROW(Utilities::parse_onnx_file(base_path_no_extension + ".onnx"));
  }

  TEST(UtilitiesTestSuit, OutputOnnxFileTest)
  {
    std::string const model_path{base_path_no_extension + ".onnx"};
    std::string const model_path_copy{base_path_no_extension + "-copy.onnx"};

    if (Utilities::file_exists(model_path_copy))
      std::filesystem::remove(model_path_copy);

    EXPECT_NO_THROW(Utilities::output_onnx_file(Utilities::parse_onnx_file(model_path), model_path_copy));

    EXPECT_TRUE(Utilities::file_exists(model_path_copy));
    std::filesystem::remove(model_path_copy);
  }

} // namespace