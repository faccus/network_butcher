list(APPEND BASE_SOURCE
        ${CMAKE_SOURCE_DIR}/src/Types/Dense_tensor.cpp
        ${CMAKE_SOURCE_DIR}/src/Utilities.cpp ${CMAKE_SOURCE_DIR}/src/General_Manager.cpp ${CMAKE_SOURCE_DIR}/src/IO_Manager.cpp ${CMAKE_SOURCE_DIR}/src/Butcher.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Onnx_importer_helpers.cpp ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Onnx_model_reconstructor_helpers.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Yaml_importer_helpers.cpp ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_importer_helpers.cpp

        ${CMAKE_SOURCE_DIR}/src/APSC/chrono.cpp)