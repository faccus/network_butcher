set(BASE_SOURCE
        ${CMAKE_SOURCE_DIR}/src/Onnx_model/onnx.pb.cc
        ${CMAKE_SOURCE_DIR}/src/Types/Dense_tensor.cpp
        ${CMAKE_SOURCE_DIR}/src/Utilities.cpp ${CMAKE_SOURCE_DIR}/src/General_Manager.cpp ${CMAKE_SOURCE_DIR}/src/IO_Manager.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Onnx_importer_helpers.cpp ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Onnx_model_reconstructor_helpers.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Yaml_importer_helpers.cpp
        ${CMAKE_SOURCE_DIR}/src/K-shortest_path/Heap_eppstein.cpp
        ${CMAKE_SOURCE_DIR}/src/Computer/Computer_flops.cpp
        ${CMAKE_SOURCE_DIR}/src/APSC/chrono.cpp
        PARENT_SCOPE)