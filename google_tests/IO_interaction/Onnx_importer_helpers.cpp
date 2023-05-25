//
// Created by faccus on 8/8/22.
//

#include "Onnx_importer_helpers.h"

#include <gtest/gtest.h>

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::io;

  onnx::ModelProto
  import_simple_model();


  TEST(OnnxImporterHelpersTests, ReadIOsValueInfoProto)
  {
    auto const  model = import_simple_model();
    auto const &graph = model.graph();

    Onnx_importer_helpers::Map_IO in_map;
    Onnx_importer_helpers::read_ios(in_map, graph.output(), {});

    auto const name = "boxes";

    EXPECT_NE(in_map.find(name), in_map.cend());
    EXPECT_FALSE(in_map.find(name)->second->initialized());

    in_map.clear();
    Onnx_importer_helpers::read_ios(in_map, graph.output(), {name});


    EXPECT_NE(in_map.find(name), in_map.cend());
    EXPECT_TRUE(in_map.find(name)->second->initialized());
  }

  TEST(OnnxImporterHelpersTests, ReadIOsTensorProto)
  {
    auto const  model = import_simple_model();
    auto const &graph = model.graph();

    Onnx_importer_helpers::Map_IO in_map;
    Onnx_importer_helpers::read_ios(in_map, graph.initializer(), {});

    auto const name = "base_net.7.branch2.1.bn.running_var";

    EXPECT_NE(in_map.find(name), in_map.cend());
    EXPECT_FALSE(in_map.find(name)->second->initialized());
    EXPECT_EQ(in_map.find(name)->second->get_shape()[0], 12);


    in_map.clear();
    Onnx_importer_helpers::read_ios(in_map, graph.initializer(), {name});

    EXPECT_NE(in_map.find(name), in_map.cend());
    EXPECT_TRUE(in_map.find(name)->second->initialized());
  }

  TEST(OnnxImporterHelpersTests, ProcessNodeIOs)
  {
    auto const  model = import_simple_model();
    auto const &graph = model.graph();

    io_collection_type<type_info_pointer> params;
    Onnx_importer_helpers::Map_IO         map;
    map.emplace("245", std::make_shared<network_butcher::types::Dense_tensor>(10, std::vector<shape_type>{1, 1, 2, 2}));

    auto const in = Onnx_importer_helpers::process_node_ios(graph.node(0).output(), params, map);

    EXPECT_EQ(in.size(), 1);
  }


  onnx::ModelProto
  import_simple_model()
  {
    std::string const model_path = Utilities::combine_path("test_data/models", "version-RFB-640-inferred.onnx");
    return Utilities::parse_onnx_file(model_path);
  }
} // namespace