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


void Analyze(const onnx::ValueInfoProto &);
void Analyze(const onnx::NodeProto &);
void PrintInputOutput(const onnx::ModelProto &);

TEST(MasterTest, Test) {
  using Type_info_pointer = std::shared_ptr<Type_info>;
  using Layers = Node<Type_info_pointer>;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const std::string model_path = "arcfaceresnet100-8-inferred.onnx";
  Graph<Layers> graph(model_path, true);
  Butcher butcher(std::move(graph));


  auto res = butcher.compute_two_slice_brute_force();

  // https://github.com/microsoft/onnxruntime/blob/master/include/onnxruntime/core/session/onnxruntime_c_api.h

  // https://github.com/microsoft/onnxruntime/blob/cee79526fd68b6fb6b09d47dae0d48b47d2f7021/docs/OperatorKernels.md

  std::cout << std::endl;
}


void Analyze(const onnx::ValueInfoProto & info) {

  auto & first_input_type = info.type();
  std::cout << info.name() << " ";

  if(first_input_type.IsInitialized()) {
      if(first_input_type.has_map_type()) {
          auto & type = first_input_type.map_type();
          std::cout << std::endl;
        }

      if(first_input_type.has_optional_type()) {
          auto & type = first_input_type.optional_type();
          std::cout << std::endl;
        }

      if(first_input_type.has_sequence_type()) {
          auto & type = first_input_type.sequence_type();
          std::cout << std::endl;
        }

      if(first_input_type.has_tensor_type()) {
          const auto & type = first_input_type.tensor_type();

          const auto elem_type = type.elem_type();

        }

      if(first_input_type.has_sparse_tensor_type()) {
          auto & type = first_input_type.sparse_tensor_type();
        }

    }

  std::cout << std::endl;
}
void Analyze(const onnx::NodeProto & node) {
  std::cout << "Node " << node.name() << std::endl;

  auto & input = node.input();
  auto & output = node.output();

  std::cout << "Input: " << std::endl;
  for(const auto & s : input)
    std::cout << s << std::endl;

  std::cout << "Output: " << std::endl;
  for(const auto & s : output)
    std::cout << s << std::endl;


  std::cout << std::endl;
}

void PrintInputOutput(const onnx::ModelProto & mod) {
  auto & graph = mod.graph();
  auto & inputs = graph.input();
  auto & outputs = graph.output();
  auto & value_info = graph.value_info();

  std::cout << "General Inputs: " << std::endl;
  for(auto & in : inputs)
    Analyze(in);

  std::cout << "General Outputs: " << std::endl;
  for(auto & out : outputs)
    Analyze(out);

  std::cout << "Value_info: " << std::endl;
  for(auto & vi : value_info)
    Analyze(vi);

  std::cout << "Nodes: " << std::endl;
  for(auto & node : graph.node())
    Analyze(node);
}



