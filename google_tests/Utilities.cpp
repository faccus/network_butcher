//
// Created by faccus on 30/08/21.
//
#include "Utilities.h"
#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;
  std::string const base_path_no_extension = "test_data/models/resnet18-v2-7-inferred";

  TEST(UtilitiesTest, VerifyProtobufVersion)
  {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
  }

  TEST(UtilitiesTest, ParseNonExistingOnnxFile)
  {
    EXPECT_THROW(Utilities::parse_onnx_file("this_model_doesnt_exists.onnx"), std::runtime_error);
  }

  TEST(UtilitiesTest, ComputeMemoryUsageFromEnum)
  {
    auto res = Utilities::compute_memory_usage_from_enum(1);
    ASSERT_EQ(res, sizeof(float));

    res = Utilities::compute_memory_usage_from_enum(-1);
    ASSERT_EQ(res, 0);
  }

  TEST(UtilitiesTest, ParseOnnxFile)
  {
    EXPECT_NO_THROW(Utilities::parse_onnx_file(base_path_no_extension + ".onnx"));
  }

  TEST(UtilitiesTest, OutputOnnxFile)
  {
    std::string const model_path{base_path_no_extension + ".onnx"};
    std::string const model_path_copy{base_path_no_extension + "-copy.onnx"};

    if (Utilities::file_exists(model_path_copy))
      std::filesystem::remove(model_path_copy);

    EXPECT_NO_THROW(Utilities::output_onnx_file(Utilities::parse_onnx_file(model_path), model_path_copy));

    EXPECT_TRUE(Utilities::file_exists(model_path_copy));
    std::filesystem::remove(model_path_copy);
  }

  TEST(UtilitiesTest, CustomToString) {
    ASSERT_EQ(Utilities::custom_to_string(501), "501");
    ASSERT_EQ(Utilities::custom_to_string(std::make_pair(501, 502)), "501 502");
    ASSERT_EQ(Utilities::custom_to_string(std::make_tuple(501, 502)), "");
  }

} // namespace