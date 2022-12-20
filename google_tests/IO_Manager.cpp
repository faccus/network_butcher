//
// Created by root on 12/04/22.
//
#include "../include/IO_Manager.h"
#include <gtest/gtest.h>

namespace
{
  TEST(IOManagerTestSuit, RegressionParamtersToExcelTest)
  {
    auto const out = network_butcher_io::IO_Manager::import_from_onnx("version-RFB-640-inferred.onnx");
    network_butcher_io::IO_Manager::old_export_network_infos_to_csv(std::get<0>(out), std::get<1>(out));
  }

  TEST(IOManagerTestSuit, ImportWeightsFromCsvTest)
  {
    auto graph =
      std::get<0>(network_butcher_io::IO_Manager::import_from_onnx("version-RFB-640-inferred.onnx", true, 3));

    network_butcher_io::IO_Manager::import_weights(network_butcher_parameters::Weight_Import_Mode::aMLLibrary,
                                                   graph,
                                                   "aMLLibrary_prediction_tegra.csv",
                                                   0);

    ASSERT_EQ(graph.get_weigth(0, {73, 74}), 0.018818040739131837);
    ASSERT_EQ(graph.get_weigth(0, {0, 1}), 0.);
  }

  TEST(IOManagerTestSuit, ImportOnnxTest)
  {
    using Input = graph_input_type;

    const std::string model_path = "version-RFB-640-inferred.onnx";
    network_butcher_io::IO_Manager::import_from_onnx(model_path);
  }
} // namespace