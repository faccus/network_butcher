//
// Created by root on 12/04/22.
//
#include "../../src/Helpers/IO_Manager.h"
#include <gtest/gtest.h>

TEST(IOManagerTestSuit, RegressionParamtersToExcelTest)
{
  auto const out = IO_Manager::import_from_onnx("version-RFB-640-inferred.onnx");
  IO_Manager::regression_parameters_to_csv(std::get<0>(out), std::get<1>(out));
}