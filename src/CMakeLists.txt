set(BASE_SOURCE
        src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc
        src/Types/Dense_tensor.cpp
        src/Utilities.cpp src/General_Manager.cpp src/IO_Manager.cpp
        src/IO_Interaction/Onnx_importer_helpers.cpp src/IO_Interaction/Onnx_model_reconstructor_helpers.cpp
        src/IO_Interaction/Yaml_importer_helpers.cpp
        src/K-shortest_path/Heap_eppstein.cpp
        src/Computer/Computer_flops.cpp
        src/APSC/chrono.cpp
        PARENT_SCOPE)
add_subdirectory(Onnx_model)