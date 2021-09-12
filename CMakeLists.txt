cmake_minimum_required(VERSION 3.20)
project(network_butcher)

set(CMAKE_CXX_STANDARD 20)

# Google tests
add_subdirectory(google_tests)

# Protobuf (dynamic library)
include(FindProtobuf)
find_package(Protobuf REQUIRED)
message(STATUS "Protobuf version: ${Protobuf_VERSION}")

# Download onnx file and create source files
add_subdirectory(src/Onnx_model)


# Turn tests on!
enable_testing()

#message( ${TESTS_SOURCE})
# main.cpp

set(TESTS_SOURCE google_tests/Helpers/Utilities.cpp google_tests/Tester.cpp google_tests/Helpers/Types/Dense_tensor.cpp)
set(LIBRARIES_SOURCE ${PROTO_HDRS} google_tests/Butcher.cpp google_tests/Network/Graph.cpp)

add_executable(network_butcher_run ${TESTS_SOURCE} ${LIBRARIES_SOURCE} src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc src/Network/Layer.h src/Network/Graph.h src/Butcher.cpp src/Butcher.h src/Network/Node.h src/Helpers/Memory_info.h src/Helpers/Types/Type_info.cpp src/Helpers/Types/Type_info.h src/Helpers/Types/Dense_tensor.cpp src/Helpers/Types/Dense_tensor.h src/Helpers/Utilities.h src/Helpers/Utilities.cpp)

target_link_libraries(network_butcher_run ${Protobuf_LIBRARIES})
target_link_libraries(network_butcher_run gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(network_butcher_run)