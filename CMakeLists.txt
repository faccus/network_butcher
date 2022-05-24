cmake_minimum_required(VERSION 3.21)
project("Network Butcher" DESCRIPTION "Just a normal butcher. A network butcher")

set(CMAKE_CXX_STANDARD 20)



option(BUILD_PROTOBUF_SOURCE "Download and compile protobuf from git" ON)
message(STATUS "Download and compile protobuf from git: " ${BUILD_PROTOBUF_SOURCE})


option(BUILD_DOC "Build documentation" OFF)
message(STATUS "Build documentation: ${BUILD_DOC}")

option(BUILD_DOC_GRAPH "Build documentation graphs" ON)
message(STATUS "Build documentation graphs: ${BUILD_DOC_GRAPH}")


option(BUILD_TESTS "Build tests" ON)
message(STATUS "Build tests: ${BUILD_TESTS}")

option(BUILD_BENCHMARK "Build benchmark tests" OFF)
message(STATUS "Build benchmark tests: ${BUILD_BENCHMARK}")


# Dependencies manager
add_subdirectory(cmake)

# Download onnx file and create source files
add_subdirectory(include)
add_subdirectory(src)

add_subdirectory(google_tests)

add_executable(network_butcher_run main.cpp ${BASE_SOURCE} ${BASE_HEADER})

target_link_libraries(network_butcher_run  ${RUN_LIBRARIES_LOCATION})

if( BUILD_TESTS )
    if( BUILD_BENCHMARK )
        set(FINAL_TESTS_SOURCE ${TESTS_SOURCE} ${BENCHMARK_TESTS_SOURCE})
    else()
        set(FINAL_TESTS_SOURCE ${TESTS_SOURCE})
    endif()

    enable_testing()

    add_executable(test_run ${FINAL_TESTS_SOURCE} ${BASE_SOURCE} ${BASE_HEADER})
    target_link_libraries(test_run ${TEST_LIBRARIES_LOCATION})
    include(GoogleTest)
    gtest_discover_tests(test_run)
endif()

add_subdirectory(doc)
