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
add_subdirectory(src)

add_subdirectory(google_tests)

add_executable(network_butcher_run main.cpp ${BASE_SOURCE} src/Helpers/IO_Manager.cpp src/Helpers/IO_Manager.h)

target_link_libraries(network_butcher_run  ${RUN_LIBRARIES_LOCATION})

if( BUILD_TESTS )
    enable_testing()
    add_executable(test_run ${TESTS_SOURCE} ${BASE_SOURCE})
    target_link_libraries(test_run ${TEST_LIBRARIES_LOCATION})
    include(GoogleTest)
    gtest_discover_tests(test_run)
endif()

add_subdirectory(doc)
