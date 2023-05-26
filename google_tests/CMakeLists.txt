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
        google_tests/K-shortest_path/KFinder.cpp
        google_tests/Butcher/Butcher.cpp
        google_tests/Butcher/Constrained_Block_Graph_Builder.cpp
        google_tests/IO_interaction/Weight_importer_helpers.cpp
        google_tests/General_Manager.cpp
)

set(
        COMPLETE_TESTS_SOURCE
        google_tests/Butcher/Butcher_complete.cpp
        google_tests/K-shortest_path/KFinder_complete.cpp
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