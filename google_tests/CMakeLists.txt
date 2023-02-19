set(
        TESTS_SOURCE
        google_tests/TestClass.h google_tests/TestClass.h
        google_tests/Types/Dense_tensor.cpp
        google_tests/Utilities.cpp
        google_tests/Onnx_interaction/Onnx_importer_helpers.cpp
        google_tests/Network/Node.cpp
        google_tests/Computer/Computer.cpp
        google_tests/Network/Graph.cpp
        google_tests/IO_Manager.cpp
        google_tests/K-shortest_path/Shortest_path_finder.cpp
        google_tests/Butcher.cpp
)

set(
        BENCHMARK_TESTS_SOURCE
        google_tests/General_Manager.cpp
        google_tests/Butcher_benchmark.cpp
)



# Add model for tests
FILE(COPY ${CMAKE_SOURCE_DIR}/models/resnet18-v2-7-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)
FILE(COPY ${CMAKE_SOURCE_DIR}/models/version-RFB-640-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)
FILE(COPY ${CMAKE_SOURCE_DIR}/models/mobilenet_v2-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)

FILE(COPY ${CMAKE_SOURCE_DIR}/models/aMLLibrary_prediction_tegra.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/models/aMLLibrary_prediction_pi.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)

FILE(COPY ${CMAKE_SOURCE_DIR}/models/official_operation_time.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_cloud.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_cloud_slow.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_edge.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)



if (BUILD_BENCHMARK)
    set(FINAL_TESTS_SOURCE ${TESTS_SOURCE} ${BENCHMARK_TESTS_SOURCE})

    FILE(COPY ${CMAKE_SOURCE_DIR}/models/test_parameters.conf  DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/test2_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/test3_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/test4_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
else ()
    set(FINAL_TESTS_SOURCE ${TESTS_SOURCE})
endif ()