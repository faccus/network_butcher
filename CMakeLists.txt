cmake_minimum_required(VERSION 3.21)
project("Network Butcher" DESCRIPTION "Just a normal butcher. A network butcher")
set(CMAKE_CXX_STANDARD 20)

# #######################################################################################################################
# ############ USER OPTIONS #############################################################################################
# #######################################################################################################################

option(BUILD_TESTS "Build tests" ON)
message(STATUS "Build tests: ${BUILD_TESTS}")

option(BUILD_BENCHMARK "Build benchmark tests" OFF)
message(STATUS "Build benchmark tests: ${BUILD_BENCHMARK}")



option(BUILD_PROTOBUF_SOURCE "Download and compile protobuf from git" ON)
message(STATUS "Download and compile protobuf from git: " ${BUILD_PROTOBUF_SOURCE})



option(USE_YAML_CPP "Use yaml-cpp" ON)
message(STATUS "It will use the yaml-cpp library to read yaml files: " ${USE_YAML_CPP})

option(BUILD_YAML_CPP_SOURCE "Download and compile yaml-cpp from git" ON)
message(STATUS "Download and compile yaml-cpp from git: " ${BUILD_YAML_CPP_SOURCE})



option(USE_PYBIND "Download and compile pybind11 from git:" ON)
message(STATUS "Download and compile pybind11 from git ${USE_PYBIND}")

option(BUILD_DOC "Build documentation" OFF)
message(STATUS "Build documentation: ${BUILD_DOC}")

option(BUILD_DOC_GRAPH "Build documentation graphs" OFF)
message(STATUS "Build documentation graphs: ${BUILD_DOC_GRAPH}")

# #######################################################################################################################
# #######################################################################################################################
# #######################################################################################################################
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

# Download onnx file and create source files
include(src/CMakeLists.txt)

# Dependencies manager
include(cmake/CMakeLists.txt)

if(USE_YAML_CPP)
    add_compile_definitions(YAML_CPP_ACTIVE)
endif()

if(USE_PYBIND)
    add_compile_definitions(PYBIND_ACTIVE)
endif()

add_executable(network_butcher src/main.cpp ${BASE_SOURCE} ${PROTO_SRCS})
target_include_directories(network_butcher PUBLIC
    include
    include/APSC
    include/Computer
    include/IO_Interaction
    include/K-shortest_path
    include/Network
    include/Traits
    include/Types
    ${EXTRA_INCLUDE_DIR}
)

target_link_libraries(network_butcher ${RUN_LIBRARIES_LOCATION})

if(BUILD_TESTS)
    include(google_tests/CMakeLists.txt)

    enable_testing()

    add_executable(test_run ${FINAL_TESTS_SOURCE} ${BASE_SOURCE})

    target_include_directories(test_run PUBLIC
        include
        include/APSC
        include/Computer
        include/IO_Interaction
        include/K-shortest_path
        include/Network
        include/Traits
        include/Types
        google_tests
        ${EXTRA_INCLUDE_DIR}
    )
    target_link_libraries(test_run ${TEST_LIBRARIES_LOCATION})

    include(GoogleTest)
    gtest_discover_tests(test_run)
endif()

if(BUILD_DOC)
    include(doc/CMakeLists.txt)
endif()