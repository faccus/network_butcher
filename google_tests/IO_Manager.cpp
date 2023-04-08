//
// Created by root on 12/04/22.
//
#include "IO_Manager.h"
#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;

  std::string base_path  = "test_data";
  std::string graph_path = Utilities::combine_path(base_path, "models/version-RFB-640-inferred.onnx");

  TEST(IOManagerTestSuit, ImportOnnxTest)
  {
    using Input = graph_input_type;

    const std::string model_path   = graph_path;
    auto const [graph, model, map] = io::IO_Manager::import_from_onnx(model_path);

    ASSERT_EQ(graph.size() - 2, map.size());

    for (auto const &node : graph.get_nodes())
      {
        auto const it = map.find(node.get_id());
        if (it != map.cend())
          EXPECT_NO_THROW(model.graph().node(it->second));
      }
  }

  TEST(IOManagerTestSuit, ImportOnnxTestNoPadding)
  {
    using Input = graph_input_type;

    const std::string model_path   = graph_path;
    auto const [graph, model, map] = io::IO_Manager::import_from_onnx(model_path, false, false);

    ASSERT_EQ(graph.size(), map.size());

    for (auto const &node : graph.get_nodes())
      {
        auto const it = map.find(node.get_id());

        EXPECT_NE(it, map.cend());
        EXPECT_NO_THROW(model.graph().node(it->second));
      }
  }

  TEST(IOManagerTestSuit, ImportOnnxTestInputPadding)
  {
    using Input = graph_input_type;

    const std::string model_path   = graph_path;
    auto const [graph, model, map] = io::IO_Manager::import_from_onnx(model_path, true, false);

    ASSERT_EQ(graph.size() - 1, map.size());

    EXPECT_EQ(map.find(graph.get_nodes()[0].get_id()), map.cend());

    for (auto node_it = ++graph.get_nodes().cbegin(); node_it != graph.get_nodes().cend(); ++node_it)
      {
        auto const it = map.find(node_it->get_id());

        EXPECT_NE(it, map.cend());
        EXPECT_NO_THROW(model.graph().node(it->second));
      }
  }

  TEST(IOManagerTestSuit, ImportOnnxTestOutputPadding)
  {
    using Input = graph_input_type;

    const std::string model_path   = graph_path;
    auto const [graph, model, map] = io::IO_Manager::import_from_onnx(model_path, false, true);

    ASSERT_EQ(graph.size() - 1, map.size());

    EXPECT_EQ(map.find(graph.get_nodes()[graph.size() - 1].get_id()), map.cend());

    for (auto node_it = ++graph.get_nodes().crbegin(); node_it != graph.get_nodes().crend(); ++node_it)
      {
        auto const it = map.find(node_it->get_id());

        EXPECT_NE(it, map.cend());
        EXPECT_NO_THROW(model.graph().node(it->second));
      }
  }

  TEST(IOManagerTestSuit, ImportExportOnnxTest)
  {
    auto const model          = io::IO_Manager::import_from_onnx(graph_path);
    auto const exported_model = Utilities::combine_path(base_path, "exported.onnx");

    if (Utilities::file_exists(exported_model))
      Utilities::file_delete(exported_model);

    EXPECT_NO_THROW(io::IO_Manager::export_to_onnx(std::get<1>(model), exported_model));

    Utilities::file_delete(exported_model);
  }


} // namespace