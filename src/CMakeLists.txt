list(APPEND BASE_SOURCE
        ${CMAKE_SOURCE_DIR}/src/Types/Dense_tensor.cpp
        ${CMAKE_SOURCE_DIR}/src/Utilities.cpp
        ${CMAKE_SOURCE_DIR}/src/General_Manager.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Manager.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Onnx_importer_helpers.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Onnx_model_reconstructor_helpers.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/Weight_Importer_Utils.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/Csv_Weight_Importer.cpp
        ${CMAKE_SOURCE_DIR}/src/APSC/chrono.cpp)

if (USE_YAML_CPP)
    list(APPEND BASE_SOURCE
            ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Yaml_importer_helpers.cpp)
endif ()

if (USE_PYBIND)
    list(APPEND BASE_SOURCE
            ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/basic_aMLLibrary_Weight_Importer.cpp
            ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/original_aMLLibrary_Weight_Importer.cpp
            ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/block_aMLLibrary_Weight_Importer.cpp)
endif ()