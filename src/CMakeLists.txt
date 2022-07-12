set(BASE_SOURCE
        src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc
        src/Helpers/Computer/Computer_memory.cpp src/Helpers/Computer/Hardware_specifications.cpp src/Helpers/Computer/Computer_time.cpp
        src/Helpers/Types/Dense_tensor.cpp
        src/Helpers/Utilities.cpp src/Helpers/General_Manager.cpp src/Helpers/IO_Manager.cpp
        src/Helpers/K-shortest_path/Heap_eppstein.cpp
        src/Helpers/APSC/chrono.cpp
        PARENT_SCOPE)
add_subdirectory(Onnx_model)