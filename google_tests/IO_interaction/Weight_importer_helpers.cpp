#include "Weight_importer_helpers.h"
#include "IO_Manager.h"

#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;

  std::string base_path = "test_data";

  std::string graph_path  = Utilities::combine_path(base_path, "models/version-RFB-640-inferred.onnx");
  std::string graph2_path = Utilities::combine_path(base_path, "models/mobilenet_v2-inferred.onnx");


  TEST(WeightImporterTestSuit, DirectImportWeightsFromCsvTest)
  {
    std::string weight_path = Utilities::combine_path(base_path, "weights/aMLLibrary_prediction_tegra.csv");
    auto        graph       = std::get<0>(io::IO_Manager::import_from_onnx(graph_path));

    io::Csv_Weight_Importer importer(
      graph, std::vector<std::string>{weight_path}, std::vector<std::string>{"pred"}, std::vector<std::size_t>{0}, ',');

    importer.import_weights([](const node_type &node) { return node.content.get_operation_id() == "conv"; });

    ASSERT_DOUBLE_EQ(graph.get_weight(0, {72, 73}), 0.018818040739131837);
  }

  TEST(WeightImporterTestSuit, MultipleImportWeightsFromCsvTest)
  {
    std::string weight_path = Utilities::combine_path(base_path, "weights/official_operation_time.csv");
    auto        graph       = std::get<0>(io::IO_Manager::import_from_onnx(graph2_path, true, true, 2));

    parameters::Device fake_1, fake_2;
    fake_1.id = 0;
    fake_2.id = 1;

    io::Csv_Weight_Importer importer(graph,
                                     std::vector<std::string>{weight_path},
                                     std::vector<std::string>{"layerinftimeedge", "layerinftimecloud"},
                                     std::vector<network_butcher::parameters::Device>{fake_1, fake_2},
                                     ',');
    importer.import_weights([](const node_type &node) { return node.content.get_operation_id() == "conv"; });

    EXPECT_DOUBLE_EQ(graph.get_weight(0, {1, 2}), 0.000177);
    ASSERT_DOUBLE_EQ(graph.get_weight(1, {1, 2}), 0.000249);
  }


} // namespace
