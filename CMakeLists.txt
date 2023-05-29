cmake_minimum_required(VERSION 3.21)
project("Network Butcher" DESCRIPTION "Just a normal butcher. A network butcher")
set(CMAKE_CXX_STANDARD 20)

# #######################################################################################################################
# ############ USER OPTIONS #############################################################################################
# #######################################################################################################################

option(BUILD_TESTS "Build tests" ON)
option(BUILD_COMPLETE_TESTS "Build complete tests" OFF)
option(ENABLE_PARALLEL "Enable parallel execution" ON)


option(BUILD_PROTOBUF_SOURCE "Download and compile protobuf from git" ON)

option(USE_PYBIND "Download and compile pybind11 from git:" ON)


option(BUILD_DOC "Build documentation" ON)

# #######################################################################################################################
# #######################################################################################################################
# #######################################################################################################################

message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build complete tests: ${BUILD_COMPLETE_TESTS}")
message(STATUS "Enable parallel execution: ${ENABLE_PARALLEL}")

message(STATUS "Download and compile protobuf from git: " ${BUILD_PROTOBUF_SOURCE})

message(STATUS "Download and compile pybind11 from git: " ${USE_PYBIND})

message(STATUS "Build documentation: ${BUILD_DOC}")


if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif ()

# Download onnx file and create source files
include(src/CMakeLists.txt)

# Dependencies manager
include(cmake/CMakeLists.txt)

if (USE_PYBIND)
    add_compile_definitions(PYBIND_ACTIVE)
endif ()

if (ENABLE_PARALLEL)
    add_compile_definitions(PARALLEL)
endif ()

add_executable(network_butcher src/main.cpp ${BASE_SOURCE} ${PROTO_SRCS})
target_include_directories(network_butcher PUBLIC
        include
        include/APSC
        include/Computer
        include/Butcher
        include/IO_Interaction
        include/IO_Interaction/Weight_Importer_Helpers
        include/K-shortest_path
        include/Network
        include/Traits
        include/Types
        ${EXTRA_INCLUDE_DIR}
        )

target_link_libraries(network_butcher ${RUN_LIBRARIES_LOCATION})

if (BUILD_TESTS)
    include(google_tests/CMakeLists.txt)

    enable_testing()

    add_executable(test_run ${TESTS_SOURCE} ${BASE_SOURCE})
    target_include_directories(test_run PUBLIC
            include
            include/APSC
            include/Computer
            include/Butcher
            include/IO_Interaction
            include/IO_Interaction/Weight_Importer_Helpers
            include/K-shortest_path
            include/Network
            include/Traits
            include/Types
            google_tests
            ${EXTRA_INCLUDE_DIR}
            )
    target_link_libraries(test_run ${TEST_LIBRARIES_LOCATION})

    add_executable(test_run_complete ${TESTS_SOURCE} ${COMPLETE_TESTS_SOURCE} ${BASE_SOURCE})
    target_include_directories(test_run_complete PUBLIC
            include
            include/APSC
            include/Computer
            include/Butcher
            include/IO_Interaction
            include/IO_Interaction/Weight_Importer_Helpers
            include/K-shortest_path
            include/Network
            include/Traits
            include/Types
            google_tests
            ${EXTRA_INCLUDE_DIR}
            )
    target_link_libraries(test_run_complete ${TEST_LIBRARIES_LOCATION})

    include(GoogleTest)

    if (BUILD_COMPLETE_TESTS)
        gtest_discover_tests(test_run_complete)
    else()
        gtest_discover_tests(test_run)
    endif ()
endif ()

if (BUILD_DOC)
    include(doc/CMakeLists.txt)
endif ()