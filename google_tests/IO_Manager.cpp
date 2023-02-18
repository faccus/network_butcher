//
// Created by root on 12/04/22.
//
#include "../include/IO_Manager.h"
#include <gtest/gtest.h>

namespace
{
  std::string base_path = "test_data";

  std::string weight_path =
    network_butcher_utilities::combine_path(base_path, "weights/aMLLibrary_prediction_tegra.csv");
  std::string graph_path = network_butcher_utilities::combine_path(base_path, "models/version-RFB-640-inferred.onnx");

  TEST(IOManagerTestSuit, RegressionParamtersToExcelTest)
  {
    auto const out = network_butcher_io::IO_Manager::import_from_onnx(graph_path);
    network_butcher_io::IO_Manager::old_export_network_infos_to_csv(std::get<0>(out), std::get<1>(out));
  }

  TEST(IOManagerTestSuit, ImportWeightsFromCsvTest)
  {
    auto graph =
      std::get<0>(network_butcher_io::IO_Manager::import_from_onnx(graph_path));

    network_butcher_io::IO_Manager::import_weights(network_butcher_parameters::Weight_Import_Mode::aMLLibrary,
                                                   graph,
                                                   weight_path,
                                                   0);

    ASSERT_EQ(graph.get_weigth(0, {73, 74}), 0.018818040739131837);
    ASSERT_EQ(graph.get_weigth(0, {0, 1}), 0.);
  }

  TEST(IOManagerTestSuit, ImportOnnxTest)
  {
    using Input = graph_input_type;

    const std::string model_path = graph_path;
    network_butcher_io::IO_Manager::import_from_onnx(model_path);
  }

  TEST(IOManagerTestSuit, ImportExportOnnxTest)
  {
    auto const model          = network_butcher_io::IO_Manager::import_from_onnx(graph_path);
    auto const exported_model = network_butcher_utilities::combine_path(base_path, "exported.onnx");

    if (network_butcher_utilities::file_exists(exported_model))
      network_butcher_utilities::file_delete(exported_model);

    network_butcher_io::IO_Manager::export_to_onnx(std::get<1>(model), exported_model);
  }
} // namespace