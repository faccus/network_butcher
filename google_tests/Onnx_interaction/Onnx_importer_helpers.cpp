//
// Created by faccus on 8/8/22.
//

#include "Onnx_importer_helpers.h"

#include <gtest/gtest.h>

namespace
{
  onnx::ModelProto
  import_simple_model();


  TEST(OnnxImporterHelpersTests, ReadIOsValueInfoProtoTest)
  {
    auto const  model = import_simple_model();
    auto const &graph = model.graph();

    network_butcher_io::Onnx_importer_helpers::Map_IO in_map;
    network_butcher_io::Onnx_importer_helpers::read_ios(in_map, graph.output(), {});

    auto const name = "boxes";

    EXPECT_NE(in_map.find(name), in_map.cend());
    EXPECT_FALSE(in_map.find(name)->second->initialized());

    in_map.clear();
    network_butcher_io::Onnx_importer_helpers::read_ios(in_map, graph.output(), {name});


    EXPECT_NE(in_map.find(name), in_map.cend());
    EXPECT_TRUE(in_map.find(name)->second->initialized());
  }

  TEST(OnnxImporterHelpersTests, ReadIOsTensorProtoTest)
  {
    auto const  model = import_simple_model();
    auto const &graph = model.graph();

    network_butcher_io::Onnx_importer_helpers::Map_IO in_map;
    network_butcher_io::Onnx_importer_helpers::read_ios(in_map, graph.initializer(), {});

    auto const name = "base_net.7.branch2.1.bn.running_var";

    EXPECT_NE(in_map.find(name), in_map.cend());
    EXPECT_FALSE(in_map.find(name)->second->initialized());
    EXPECT_EQ(in_map.find(name)->second->get_shape()[0], 12);


    in_map.clear();
    network_butcher_io::Onnx_importer_helpers::read_ios(in_map, graph.initializer(), {name});

    EXPECT_NE(in_map.find(name), in_map.cend());
    EXPECT_TRUE(in_map.find(name)->second->initialized());
  }

  TEST(OnnxImporterHelpersTests, ProcessNodeIOsTest)
  {
    auto const  model = import_simple_model();
    auto const &graph = model.graph();

    io_collection_type<type_info_pointer>             params;
    network_butcher_io::Onnx_importer_helpers::Map_IO map;
    map.emplace("245", std::make_shared<network_butcher_types::Dense_tensor>(10, std::vector<shape_type>{1, 1, 2, 2}));

    auto const in = network_butcher_io::Onnx_importer_helpers::process_node_ios(graph.node(0).output(), params, map);

    EXPECT_EQ(in.size(), 1);
  }


  onnx::ModelProto
  import_simple_model()
  {
    std::string const model_path = Utilities::combine_path("test_data/models", "version-RFB-640-inferred.onnx");
    return Utilities::parse_onnx_file(model_path);
  }
} // namespace