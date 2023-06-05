set(
        TESTS_SOURCE
        google_tests/Types/dense_tensor.cpp
        google_tests/utilities.cpp
        google_tests/IO_interaction/onnx_importer_helpers.cpp
        google_tests/Network/node.cpp
        google_tests/Computer/computer_memory.cpp
        google_tests/Network/graph.cpp
        google_tests/io_manager.cpp
        google_tests/K-shortest_path/shortest_path_finder.cpp
        google_tests/K-shortest_path/kfinder.cpp
        google_tests/K-shortest_path/heap_eppstein.cpp
        google_tests/Butcher/butcher.cpp
        google_tests/Butcher/constrained_block_graph_builder.cpp
        google_tests/IO_interaction/weight_importer_helpers.cpp
        google_tests/general_manager.cpp
)

set(
        COMPLETE_TESTS_SOURCE
        google_tests/Butcher/butcher_complete.cpp
        google_tests/K-shortest_path/kfinder_complete.cpp
)

file(GLOB TMP_KFINDER
        "${CMAKE_SOURCE_DIR}/test_data/kfinder/in/*.in"
        )
file(COPY ${TMP_KFINDER} DESTINATION ${CMAKE_BINARY_DIR}/test_data/kfinder/in)


file(GLOB TMP_KFINDER
        "${CMAKE_SOURCE_DIR}/test_data/kfinder/out/*.out"
        )
file(COPY ${TMP_KFINDER} DESTINATION ${CMAKE_BINARY_DIR}/test_data/kfinder/out)


file(GLOB TMP_MODELS
        "${CMAKE_SOURCE_DIR}/test_data/models/*.onnx"
        )
file(COPY ${TMP_MODELS} DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)


file(GLOB TMP_WEIGHTS
        "${CMAKE_SOURCE_DIR}/test_data/weights/*.csv"
        )
file(COPY ${TMP_WEIGHTS} DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)


file(GLOB TMP_CONFIGS
        "${CMAKE_SOURCE_DIR}/test_data/configs/*.conf"
        )
file(COPY ${TMP_CONFIGS} DESTINATION ${CMAKE_BINARY_DIR}/test_data/configs)


file(GLOB TMP_PICKLE_MODELS
        "${CMAKE_SOURCE_DIR}/test_data/aMLLibrary_data/models/*.pickle"
        )
file(COPY ${TMP_PICKLE_MODELS} DESTINATION ${CMAKE_BINARY_DIR}/test_data/aMLLibrary_data/models)