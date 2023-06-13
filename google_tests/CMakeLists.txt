set(
        TESTS_SOURCE
        Types/dense_tensor.cpp
        utilities.cpp
        IO_interaction/onnx_importer_helpers.cpp
        Network/node.cpp
        Computer/computer_memory.cpp
        Network/graph.cpp
        io_manager.cpp
        K-shortest_path/shortest_path_finder.cpp
        K-shortest_path/heap_eppstein.cpp
        K-shortest_path/kfinder.cpp
        Butcher/butcher.cpp
        Butcher/constrained_block_graph_builder.cpp
        IO_interaction/weight_importer_helpers.cpp
        general_manager.cpp
)

set(
        COMPLETE_TESTS_SOURCE
        Butcher/butcher_complete.cpp
        K-shortest_path/kfinder_complete.cpp
)

# Add google test library from git (as suggested by the documentation)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})


set(TEST_LIBRARIES_LOCATION gtest gtest_main)

add_executable(test_run ${TESTS_SOURCE})
target_link_libraries(test_run PRIVATE network_butcher ${TEST_LIBRARIES_LOCATION})
target_compile_features(test_run PRIVATE cxx_std_20)


add_executable(test_run_complete ${TESTS_SOURCE} ${COMPLETE_TESTS_SOURCE})
target_link_libraries(test_run_complete PRIVATE network_butcher ${TEST_LIBRARIES_LOCATION})
target_compile_features(test_run_complete PRIVATE cxx_std_20)

include(GoogleTest)

if (DISCOVER_COMPLETE_TESTS)
    gtest_discover_tests(test_run_complete)
elseif (DISCOVER_TESTS)
    gtest_discover_tests(test_run)
endif ()


file(GLOB TMP_KFINDER
        "${CMAKE_SOURCE_DIR}/test_data/kfinder/in/*.in"
        )
file(COPY ${TMP_KFINDER} DESTINATION ${CMAKE_BINARY_DIR}/google_tests/test_data/kfinder/in)


file(GLOB TMP_KFINDER
        "${CMAKE_SOURCE_DIR}/test_data/kfinder/out/*.out"
        )
file(COPY ${TMP_KFINDER} DESTINATION ${CMAKE_BINARY_DIR}/google_tests/test_data/kfinder/out)


file(GLOB TMP_MODELS
        "${CMAKE_SOURCE_DIR}/test_data/models/*.onnx"
        )
file(COPY ${TMP_MODELS} DESTINATION ${CMAKE_BINARY_DIR}/google_tests/test_data/models)


file(GLOB TMP_WEIGHTS
        "${CMAKE_SOURCE_DIR}/test_data/weights/*.csv"
        )
file(COPY ${TMP_WEIGHTS} DESTINATION ${CMAKE_BINARY_DIR}/google_tests/test_data/weights)


file(GLOB TMP_CONFIGS
        "${CMAKE_SOURCE_DIR}/test_data/configs/*.conf"
        )
file(COPY ${TMP_CONFIGS} DESTINATION ${CMAKE_BINARY_DIR}/google_tests/test_data/configs)


file(GLOB TMP_PICKLE_MODELS
        "${CMAKE_SOURCE_DIR}/test_data/aMLLibrary_data/models/*.pickle"
        )
file(COPY ${TMP_PICKLE_MODELS} DESTINATION ${CMAKE_BINARY_DIR}/google_tests/test_data/aMLLibrary_data/models)