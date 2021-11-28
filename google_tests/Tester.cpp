//
// Created by faccus on 30/08/21.
//

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

#include "../src/Butcher.h"
#include "../src/Helpers/Utilities.h"
#include "../src/Helpers/Types/Dense_tensor.h"
#include "../src/Onnx_model/onnx.pb.h"



TEST(MasterTest, Test) {
  using Input = graph_input_type;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "resnet18-v2-7-inferred.onnx";
  Graph<Input> graph(model_path, true);
  Butcher butcher(std::move(graph));

  auto res = butcher.compute_two_slice_brute_force();


  std::cout << std::endl;
}





