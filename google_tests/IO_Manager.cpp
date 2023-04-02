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

    const std::string model_path = graph_path;
    io::IO_Manager::import_from_onnx(model_path);
  }

  TEST(IOManagerTestSuit, ImportExportOnnxTest)
  {
    auto const model          = io::IO_Manager::import_from_onnx(graph_path);
    auto const exported_model = Utilities::combine_path(base_path, "exported.onnx");

    if (Utilities::file_exists(exported_model))
      Utilities::file_delete(exported_model);

    io::IO_Manager::export_to_onnx(std::get<1>(model), exported_model);
  }


} // namespace