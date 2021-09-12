cmake_minimum_required(VERSION 3.20)
project(network_butcher)

set(CMAKE_CXX_STANDARD 20)

option(BUILD_PROTOBUF_SOURCE "Download and compile protobuf from git" ON)
message(STATUS "Download and compile protobuf from git: " ${BUILD_PROTOBUF_SOURCE})

# Dependencies manager
add_subdirectory(cmake)


# Download onnx file and create source files
add_subdirectory(src/Onnx_model)

# Turn tests on!
enable_testing()

set(TESTS_SOURCE google_tests/Butcher.cpp google_tests/Network/Graph.cpp google_tests/Helpers/Utilities.cpp google_tests/Tester.cpp google_tests/Helpers/Types/Dense_tensor.cpp)

add_executable(network_butcher_run ${TESTS_SOURCE} ${LIBRARIES_SOURCE} src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc src/Network/Layer.h src/Network/Graph.h src/Butcher.cpp src/Butcher.h src/Network/Node.h src/Helpers/Memory_info.h src/Helpers/Types/Type_info.cpp src/Helpers/Types/Type_info.h src/Helpers/Types/Dense_tensor.cpp src/Helpers/Types/Dense_tensor.h src/Helpers/Utilities.h src/Helpers/Utilities.cpp)

#target_include_directories(network_butcher_run PUBLIC ${protobuf_SOURCE_DIR}/src/google/protobuf)
target_link_libraries(network_butcher_run  ${PROTOBUF_LIBRARY_LOCATION})
target_link_libraries(network_butcher_run gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(network_butcher_run)