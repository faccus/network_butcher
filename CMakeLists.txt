cmake_minimum_required(VERSION 3.20)
project("Network Butcher" DESCRIPTION "Just a normal butcher. A network butcher")

set(CMAKE_CXX_STANDARD 20)

option(BUILD_PROTOBUF_SOURCE "Download and compile protobuf from git" ON)
message(STATUS "Download and compile protobuf from git: " ${BUILD_PROTOBUF_SOURCE})
option(BUILD_DOC "Build documentation" ON)
option(BUILD_DOC_GRAPH "Build documentation graphs" OFF)
message(STATUS "Build documentation: ${BUILD_DOC}")
option(BUILD_TESTS "Build tests" ON)
message(STATUS "Build tests: ${BUILD_TESTS}")

# Dependencies manager
add_subdirectory(cmake)

# Download onnx file and create source files
add_subdirectory(src/Onnx_model)

add_subdirectory(google_tests)

add_executable(network_butcher_run main.cpp
        src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc
        src/Network/Node.h src/Network/Graph.h src/Butcher.cpp src/Butcher.h src/Network/Computer.h  src/Network/Computer.cpp
        src/Helpers/Types/Type_info.h src/Helpers/Types/Dense_tensor.cpp src/Helpers/Types/Dense_tensor.h
        src/Helpers/Utilities.h src/Helpers/Utilities.cpp
        src/Helpers/Traits/Graph_traits.h src/Helpers/Traits/Node_traits.h src/Helpers/Traits/Network_traits.h src/Helpers/Traits/Basic_traits.h src/Helpers/Traits/Type_info_traits.h)

target_link_libraries(network_butcher_run  ${RUN_LIBRARIES_LOCATION})

if( BUILD_TESTS )
    enable_testing()
    add_executable(test_run ${TESTS_SOURCE} ${BASE_SOURCE} google_tests/TestClass.h)
    target_link_libraries(test_run ${TEST_LIBRARIES_LOCATION})
    include(GoogleTest)
    gtest_discover_tests(test_run)
endif()

add_subdirectory(doc)
