//
// Created by faccus on 8/8/22.
//

#include "../../include/Onnx_interaction/Onnx_importer_helpers.h"

#include <gtest/gtest.h>

namespace
{
  onnx::TypeProto *
  construct_type_tensor_proto();

  onnx::ValueInfoProto
  construct_value_info_proto();

  onnx::TensorProto
  construct_tensor_proto();

  TEST(OnnxImporterHelpersTests, ReadIOsValueInfoProtoTest)
  {
    google::protobuf::RepeatedPtrField<onnx::ValueInfoProto> rep_field;

    rep_field.Add(construct_value_info_proto());


    network_butcher_io::Onnx_importer_helpers::Map_IO in_map;
    network_butcher_io::Onnx_importer_helpers::read_ios(in_map, rep_field, {});

    EXPECT_TRUE(in_map.find("Test") != in_map.cend());
    EXPECT_FALSE(in_map.find("Test")->second->initialized());

    in_map.clear();
    network_butcher_io::Onnx_importer_helpers::read_ios(in_map, rep_field, {"Test"});

    EXPECT_TRUE(in_map.find("Test") != in_map.cend());
    EXPECT_TRUE(in_map.find("Test")->second->initialized());
  }




  onnx::TypeProto *
  construct_type_tensor_proto()
  {
    auto                  *typeProto = new onnx::TypeProto();
    onnx::TypeProto_Tensor tensor;
    onnx::TensorShapeProto shapeProto;

    for (int i = 0; i < 4; ++i)
      {
        auto dim = shapeProto.add_dim();
        dim->set_dim_value(i + 1);
      }

    tensor.set_allocated_shape(new onnx::TensorShapeProto(std::move(shapeProto)));
    tensor.set_elem_type(onnx::TensorProto_DataType_INT64);

    typeProto->set_allocated_tensor_type(new onnx::TypeProto_Tensor(std::move(tensor)));

    return typeProto;
  }

  onnx::ValueInfoProto
  construct_value_info_proto()
  {
    onnx::ValueInfoProto value;

    value.set_allocated_type(construct_type_tensor_proto());
    value.set_name("Test");

    return value;
  }
} // namespace