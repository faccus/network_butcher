//
// Created by root on 12/04/22.
//
#include "../../src/Helpers/IO_Manager.h"
#include <gtest/gtest.h>

TEST(IOManagerTestSuit, RegressionParamtersToExcelTest)
{
  IO_Manager::regression_parameters_to_excel("version-RFB-640-inferred.onnx");
}