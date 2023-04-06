set(
        TESTS_SOURCE
        google_tests/TestClass.h google_tests/TestClass.h
        google_tests/Types/Dense_tensor.cpp
        google_tests/Utilities.cpp
        google_tests/IO_interaction/Onnx_importer_helpers.cpp
        google_tests/Network/Node.cpp
        google_tests/Computer/Computer_memory.cpp
        google_tests/Network/Graph.cpp
        google_tests/IO_Manager.cpp
        google_tests/K-shortest_path/Shortest_path_finder.cpp
        google_tests/Butcher.cpp
        google_tests/IO_interaction/Weight_importer_helpers.cpp
        google_tests/General_Manager.cpp
)

set(
        BENCHMARK_TESTS_SOURCE
        google_tests/Butcher_benchmark.cpp
)


# Add model for tests
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/models/resnet18-v2-7-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/models/version-RFB-640-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/models/mobilenet_v2-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)

FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/weights/aMLLibrary_prediction_tegra.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/weights/aMLLibrary_prediction_pi.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/weights/official_operation_time.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/weights/operation_time_cloud.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/weights/operation_time_cloud_slow.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/weights/operation_time_edge.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/weights/resnetLayerInfTimes.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)

FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/configs/test_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/configs/test2_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/configs/test3_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/configs/test4_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/configs/test5_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/configs/test6_parameters.conf DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)

FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/aMLLibrary_data/models/test1_1.pickle DESTINATION ${CMAKE_BINARY_DIR}/test_data/aMLLibrary_data/models)
FILE(COPY ${CMAKE_SOURCE_DIR}/test_data/aMLLibrary_data/models/test1_2.pickle DESTINATION ${CMAKE_BINARY_DIR}/test_data/aMLLibrary_data/models)