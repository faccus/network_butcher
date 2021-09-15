add_subdirectory(Onnx_model)

set(TMP_BASE_SOURCE src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc src/Network/Graph.h src/Butcher.cpp src/Butcher.h src/Network/Node.h src/Helpers/Types/Type_info.cpp src/Helpers/Types/Type_info.h src/Helpers/Types/Dense_tensor.cpp src/Helpers/Types/Dense_tensor.h src/Helpers/Utilities.h src/Helpers/Utilities.cpp)
set(BASE_SOURCE ${TMP_BASE_SOURCE} PARENT_SCOPE)