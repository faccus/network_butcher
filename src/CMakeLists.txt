list(APPEND BASE_SOURCE
        ${CMAKE_SOURCE_DIR}/src/Types/dense_tensor.cpp
        ${CMAKE_SOURCE_DIR}/src/utilities.cpp
        ${CMAKE_SOURCE_DIR}/src/general_manager.cpp
        ${CMAKE_SOURCE_DIR}/src/io_manager.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/onnx_importer_helpers.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/onnx_model_reconstructor_helpers.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/weight_importer_utils.cpp
        ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/csv_weight_importer.cpp
        ${CMAKE_SOURCE_DIR}/src/APSC/chrono.cpp)

if (USE_PYBIND)
    list(APPEND BASE_SOURCE
            ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/abstract_aMLLibrary_weight_importer.cpp
            ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/original_aMLLibrary_weight_importer.cpp
            ${CMAKE_SOURCE_DIR}/src/IO_Interaction/Weight_Importer_Helpers/block_aMLLibrary_weight_importer.cpp)
endif ()