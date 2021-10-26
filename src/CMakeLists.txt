set(BASE_SOURCE src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc
        src/Network/Node.h src/Network/Graph.h src/Butcher.cpp src/Butcher.h src/Network/Computer.h src/Network/Computer.cpp
        src/Helpers/Types/Type_info.h src/Helpers/Types/Dense_tensor.cpp src/Helpers/Types/Dense_tensor.h
        src/Helpers/Utilities.h src/Helpers/Utilities.cpp src/Helpers/K-shortest_path/ksp.h
        src/Helpers/Traits/Graph_traits.h src/Helpers/Traits/Node_traits.h src/Helpers/Traits/Network_traits.h src/Helpers/Traits/Basic_traits.h src/Helpers/Traits/Type_info_traits.h
        PARENT_SCOPE)
add_subdirectory(Onnx_model)