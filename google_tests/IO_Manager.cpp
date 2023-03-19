//
// Created by root on 12/04/22.
//
#include "IO_Manager.h"
#include <gtest/gtest.h>

namespace
{
  std::string base_path = "test_data";

  std::string graph_path  = Utilities::combine_path(base_path, "models/version-RFB-640-inferred.onnx");
  std::string graph2_path = Utilities::combine_path(base_path, "models/mobilenet_v2-inferred.onnx");

  TEST(IOManagerTestSuit, RegressionParamtersToExcelTest)
  {
    auto const out = network_butcher_io::IO_Manager::import_from_onnx(graph_path);
    network_butcher_io::IO_Manager::old_export_network_infos_to_csv(std::get<0>(out), std::get<1>(out));
  }

  TEST(IOManagerTestSuit, DirectImportWeightsFromCsvTest)
  {
    std::string weight_path = Utilities::combine_path(base_path, "weights/aMLLibrary_prediction_tegra.csv");
    auto        graph       = std::get<0>(network_butcher_io::IO_Manager::import_from_onnx(graph_path));

    network_butcher_io::Weight_importer_helpers::import_weights_aMLLibrary_direct_read(
      graph, 0, weight_path, [](const node_type &node) { return node.content.get_operation_id() == "conv"; });

    ASSERT_DOUBLE_EQ(graph.get_weight(0, {72, 73}), 0.018818040739131837);
  }

  TEST(IOManagerTestSuit, MultipleImportWeightsFromCsvTest)
  {
    std::string weight_path = Utilities::combine_path(base_path, "weights/official_operation_time.csv");
    auto        graph       = std::get<0>(network_butcher_io::IO_Manager::import_from_onnx(graph2_path));

    network_butcher_parameters::Device fake_1, fake_2;
    fake_1.id = 0;
    fake_2.id = 1;

    network_butcher_io::Weight_importer_helpers::import_weights_direct_read(
      graph, weight_path, {fake_1, fake_2}, {"layerinftimeedge", "layerinftimecloud"}, ',', [](const node_type &node) {
        return node.content.get_operation_id() == "conv";
      });

    EXPECT_DOUBLE_EQ(graph.get_weight(0, {1, 2}), 0.000177);
    ASSERT_DOUBLE_EQ(graph.get_weight(0, {1, 2}), 0.000249);
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
    auto const exported_model = Utilities::combine_path(base_path, "exported.onnx");

    if (Utilities::file_exists(exported_model))
      Utilities::file_delete(exported_model);

    network_butcher_io::IO_Manager::export_to_onnx(std::get<1>(model), exported_model);
  }


} // namespace