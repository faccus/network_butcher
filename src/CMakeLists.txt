set(BASE_SOURCE
        src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc
        src/Computer/Hardware_specifications.cpp src/Computer/Computer_time.cpp
        src/Types/Dense_tensor.cpp
        src/Utilities.cpp src/General_Manager.cpp src/IO_Manager.cpp src/Onnx_importer.cpp src/Model_reconstructor.cpp
        src/K-shortest_path/Heap_eppstein.cpp
        src/APSC/chrono.cpp
        PARENT_SCOPE)
add_subdirectory(Onnx_model)