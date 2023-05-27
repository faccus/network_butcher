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


  TEST(IOManagerTest, Parameters)
  {
    auto const params = io::IO_Manager::read_parameters("test_data/configs/test6_parameters.conf");

    ASSERT_EQ(params.model_params.model_name, "ResNet");
    ASSERT_EQ(params.model_params.model_path, "test_data/models/resnet18-v2-7-inferred.onnx");
    ASSERT_EQ(params.model_params.export_directory, "ksp_result6");

    ASSERT_EQ(params.ksp_params.K, 12);
    ASSERT_EQ(params.ksp_params.method, parameters::KSP_Method::Lazy_Eppstein);
    ASSERT_EQ(params.block_graph_generation_params.starting_device_id, 0);
    ASSERT_EQ(params.block_graph_generation_params.ending_device_id, 0);

    ASSERT_EQ(params.block_graph_generation_params.memory_constraint, true);
    ASSERT_EQ(params.block_graph_generation_params.memory_constraint_type,
              parameters::Memory_Constraint_Type::Preload_Parameters);
    ASSERT_EQ(params.devices.size(), 2);

    ASSERT_EQ(params.weights_params.weight_import_mode, parameters::Weight_Import_Mode::aMLLibrary_block);

    ASSERT_EQ(params.aMLLibrary_params.aMLLibrary_inference_variables.size(), 2);
    ASSERT_EQ(params.aMLLibrary_params.aMLLibrary_inference_variables[0], "1stInfTime");
    ASSERT_EQ(params.aMLLibrary_params.aMLLibrary_inference_variables[1], "2ndInfTime");

    std::vector<std::string> vect{"tensorLength", "networkingTime", "NrParameters", "NrNodes", "Memory", "MACs"};
    ASSERT_EQ(params.aMLLibrary_params.aMLLibrary_csv_features.size(), vect.size());
    ASSERT_EQ(params.aMLLibrary_params.aMLLibrary_csv_features, vect);

    for (std::size_t i = 0; i < params.devices.size(); ++i)
      ASSERT_EQ(params.devices[i].id, i);

    memory_type gb = 1024 * 1024 * 1024;

    ASSERT_EQ(params.devices[0].name, "RPI");
    ASSERT_EQ(params.devices[0].maximum_memory, gb);
    ASSERT_EQ(params.devices[0].weights_path, "test_data/aMLLibrary_data/models/test1_1.pickle");

    ASSERT_EQ(params.devices[1].name, "Tegra");
    ASSERT_EQ(params.devices[1].maximum_memory, 32 * gb);
    ASSERT_EQ(params.devices[1].weights_path, "test_data/aMLLibrary_data/models/test1_2.pickle");

    ASSERT_TRUE(params.weights_params.bandwidth->check_weight(0, std::make_pair(0, 1)));

    auto const &[bandwidth, access] = params.weights_params.bandwidth->get_weight(0, std::make_pair(0, 1));

    ASSERT_EQ(bandwidth, 18.88);
    ASSERT_EQ(access, 0.005);

    ASSERT_FALSE(params.weights_params.bandwidth->get_input_nodes(0).contains(1));
    auto const &[in_bandwidth, in_access] = params.weights_params.bandwidth->get_weight(1, std::make_pair(1, 0));
    ASSERT_EQ(in_bandwidth, 100000.);
    ASSERT_EQ(in_access, 1.);

    auto const &[out_bandwidth, out_access] = params.weights_params.bandwidth->get_weight(2, std::make_pair(1, 0));
    ASSERT_EQ(out_bandwidth, 200000.);
    ASSERT_EQ(out_access, 2.);

    ASSERT_FALSE(params.weights_params.bandwidth->check_weight(0, std::make_pair(1, 0)));
    ASSERT_FALSE(params.weights_params.bandwidth->check_weight(1, std::make_pair(0, 1)));
    ASSERT_FALSE(params.weights_params.bandwidth->check_weight(2, std::make_pair(0, 1)));
  }

  TEST(IOManagerTest, ImportOnnx)
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

  TEST(IOManagerTest, ImportOnnxTestNoPadding)
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

  TEST(IOManagerTest, ImportOnnxTestInputPadding)
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

  TEST(IOManagerTest, ImportOnnxTestOutputPadding)
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

  TEST(IOManagerTest, ImportExportOnnx)
  {
    auto const model          = io::IO_Manager::import_from_onnx(graph_path);
    auto const exported_model = Utilities::combine_path(base_path, "exported.onnx");

    if (Utilities::file_exists(exported_model))
      Utilities::file_delete(exported_model);

    EXPECT_NO_THROW(io::IO_Manager::export_to_onnx(std::get<1>(model), exported_model));

    Utilities::file_delete(exported_model);
  }


} // namespace