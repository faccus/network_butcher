cmake_minimum_required(VERSION 3.21)
project(
        "Network Butcher"
        VERSION 1.0
        DESCRIPTION "Just a normal butcher. A network butcher"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

########################################################################################################################
############# USER OPTIONS #############################################################################################
########################################################################################################################

option(BUILD_PROTOBUF_SOURCE "Download and compile protobuf from github" ON)

option(USE_GOOGLETEST "Enable target for tests (download automatically GoogleTest from github)" ON)
option(DISCOVER_TESTS "Let CTest discover the tests" ON)
option(DISCOVER_COMPLETE_TESTS "Let CTest discover the complete tests" ON)

option(ENABLE_NETWORK_BUTCHER_PARALLEL "Enable parallel execution" ON)
option(USE_OPENMP "Use OpenMP (if OFF, it will use TBB)" ON)

option(USE_PYBIND "Use pybind11 (downloaded automatically from github)" ON)

option(BUILD_DOC "Build documentation" ON)

########################################################################################################################
########################################################################################################################
########################################################################################################################

message(STATUS "Use GoogleTest: ${USE_GOOLETEST}")
message(STATUS "Let CTest discover the tests: ${BUILD_TESTS}")
message(STATUS "Let CTest discover the complete tests: ${BUILD_COMPLETE_TESTS}")

message(STATUS "Enable parallel execution: ${ENABLE_NETWORK_BUTCHER_PARALLEL}")
message(STATUS "Use OpenMP (if OFF, it will use TBB): ${USE_OPENMP}")


message(STATUS "Download and compile protobuf from git: " ${BUILD_PROTOBUF_SOURCE})

message(STATUS "Use pybind11 (downloaded automatically from github): " ${USE_PYBIND})

message(STATUS "Build documentation: ${BUILD_DOC}")


if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24.0")
    # Basically, files will be downloaded again based on download date/time, not extraction.
    cmake_policy(SET CMP0135 NEW)
endif ()



# Library code
add_subdirectory(src)

# Mains code
add_subdirectory(mains)

# Documentation
if (BUILD_DOC)
    find_package(Doxygen OPTIONAL_COMPONENTS dot mscgen dia) # doxygen, graphviz

    if (DOXYGEN_FOUND)
        include(doc/CMakeLists.txt)
    elseif ()
        message(STATUS "Missing doxygen: I will not be able to produce documentation files!")
    endif ()
endif ()

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND USE_GOOGLETEST)
    add_subdirectory(google_tests)
endif()