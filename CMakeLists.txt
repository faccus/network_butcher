cmake_minimum_required(VERSION 3.20)
project(network_butcher)

set(CMAKE_CXX_STANDARD 20)

# Google tests
add_subdirectory(google_tests)

# Protobuf (dynamic library)
find_package(Protobuf REQUIRED)


# Onnx protobuf model download and compilation
set(onnx_model_file_url "https://raw.githubusercontent.com/onnx/onnx/master/onnx/onnx.proto3")
set(onnx_model_file_dir "src/Onnx_model")
set(onnx_model_file_path "${onnx_model_file_dir}/onnx")
make_directory(${onnx_model_file_dir})
if (NOT EXISTS "${onnx_model_file_path}") # https://raw.githubusercontent.com/onnx/onnx/master/onnx/onnx-ml.proto3
    file(DOWNLOAD ${onnx_model_file_url} ${onnx_model_file_path} SHOW_PROGRESS)
endif()

#if (NOT EXISTS "${onnx_model_file_dir}/onnx.pb.h")
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS onnx)
#endif()

# Turn tests on!
enable_testing()

#message( ${TESTS_SOURCE})
# main.cpp

set(TESTS_SOURCE google_tests/Helpers/Utilities.cpp google_tests/Tester.cpp google_tests/Helpers/Types/Dense_tensor.cpp)
set(LIBRARIES_SOURCE ${PROTO_HDRS} )

add_executable(network_butcher_run ${TESTS_SOURCE} ${LIBRARIES_SOURCE}  ${onnx_model_file_dir}/onnx.pb.h ${onnx_model_file_dir}/onnx.pb.cc src/Network/Layer.h src/Network/Graph.h src/Butcher.cpp src/Butcher.h src/Network/Node.h src/Helpers/Memory_info.h src/Helpers/Types/Type_info.cpp src/Helpers/Types/Type_info.h src/Helpers/Types/Dense_tensor.cpp src/Helpers/Types/Dense_tensor.h src/Helpers/Utilities.h src/Helpers/Utilities.cpp)

target_link_libraries(network_butcher_run ${Protobuf_LIBRARIES})
target_link_libraries(network_butcher_run gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(network_butcher_run)