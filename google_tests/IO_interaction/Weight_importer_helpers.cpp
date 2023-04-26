#include "IO_Manager.h"
#include "Weight_importers.h"

#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;

  std::string base_path = "test_data";

  std::string graph_path  = Utilities::combine_path(base_path, "models/version-RFB-640-inferred.onnx");
  std::string graph2_path = Utilities::combine_path(base_path, "models/mobilenet_v2-inferred.onnx");

  block_graph_type
  simple_graph_generator();


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

  TEST(WeightImporterTestSuit, SingleBlockImportWeightsFromCsvTest)
  {
    std::string weight_path = Utilities::combine_path(base_path, "weights/sample_weights.csv");

    auto graph = simple_graph_generator();

    parameters::Device fake_1, fake_2;
    fake_1.id = 0;
    fake_2.id = 1;

    io::Csv_Weight_Importer importer(graph,
                                     std::vector<std::string>{weight_path},
                                     std::vector<std::string>{"fake_weight_1", "fake_weight_2"},
                                     std::vector<network_butcher::parameters::Device>{fake_1, fake_2},
                                     ',');
    importer.import_weights();

    EXPECT_DOUBLE_EQ(graph.get_weight({1, 3}), 5.1);
    EXPECT_DOUBLE_EQ(graph.get_weight({1, 4}), 5.2);

    EXPECT_DOUBLE_EQ(graph.get_weight({0, 1}), 1234.1);
    EXPECT_DOUBLE_EQ(graph.get_weight({0, 2}), 1234.2);
  }

  TEST(WeightImporterTestSuit, MultipleBlockImportWeightsFromCsvTest)
  {
    std::string weight_path = Utilities::combine_path(base_path, "weights/sample_weights.csv");

    auto graph = simple_graph_generator();

    parameters::Device fake_1, fake_2;
    fake_1.id = 0;
    fake_2.id = 1;

    io::Csv_Weight_Importer importer(graph,
                                     std::vector<std::string>{weight_path, weight_path},
                                     std::vector<std::string>{"fake_weight_1", "fake_weight_2"},
                                     std::vector<network_butcher::parameters::Device>{fake_1, fake_2},
                                     ',');
    importer.import_weights();

    EXPECT_DOUBLE_EQ(graph.get_weight({1, 3}), 5.1);
    EXPECT_DOUBLE_EQ(graph.get_weight({1, 4}), 5.2);

    EXPECT_DOUBLE_EQ(graph.get_weight({0, 1}), 1234.1);
    EXPECT_DOUBLE_EQ(graph.get_weight({0, 2}), 1234.2);
  }

  block_graph_type
  simple_graph_generator()
  {
    std::vector<block_graph_type::Node_Type> nodes;
    block_graph_type::Dependencies_Type      deps(8);

    nodes.emplace_back(std::pair{0, nullptr});
    deps[0].second.insert({1, 2});


    nodes.emplace_back(std::pair{0, nullptr});
    deps[1].first.insert({0});
    deps[1].second.insert({3, 4});

    nodes.emplace_back(std::pair{1, nullptr});
    deps[2].first.insert({0});
    deps[2].second.insert({3, 4});


    nodes.emplace_back(std::pair{0, nullptr});
    deps[3].first.insert({1, 2});
    deps[3].second.insert({5, 6});

    nodes.emplace_back(std::pair{1, nullptr});
    deps[4].first.insert({1, 2});
    deps[4].second.insert({5, 6});


    nodes.emplace_back(std::pair{0, nullptr});
    deps[5].first.insert({3, 4});
    deps[5].second.insert({7});

    nodes.emplace_back(std::pair{1, nullptr});
    deps[6].first.insert({3, 4});
    deps[6].second.insert({7});


    nodes.emplace_back(std::pair{0, nullptr});
    deps[7].first.insert({5, 6});


    return block_graph_type(std::move(nodes), std::move(deps));
  }

} // namespace
