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

TEST(IOManagerTestSuit, ImportWeightsFromCsvTest) {
  auto graph = std::get<0>(
    IO_Manager::import_from_onnx("version-RFB-640-inferred.onnx", true, 3));
  IO_Manager::import_weights_from_csv(graph, 0, "prediction.csv");

  std::cout << graph.get_weigth(0, {0,1}) << std::endl;
}