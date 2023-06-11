cmake_minimum_required(VERSION 3.21)
project("Network Butcher" DESCRIPTION "Just a normal butcher. A network butcher")
set(CMAKE_CXX_STANDARD 20)

########################################################################################################################
############# USER OPTIONS #############################################################################################
########################################################################################################################

option(BUILD_PROTOBUF_SOURCE "Download and compile protobuf from github" ON)

option(USE_GOOGLETEST "Enable target for tests (download automatically GoogleTest from github)" ON)
option(DISCOVER_TESTS "Let CTest discover the tests" ON)
option(DISCOVER_COMPLETE_TESTS "Let CTest discover the complete tests" ON)

option(ENABLE_PARALLEL "Enable parallel execution" ON)
option(USE_OPENMP "Use OpenMP (if OFF, it will use TBB)" ON)

option(USE_PYBIND "Use pybind11 (downloaded automatically from github)" ON)

option(BUILD_DOC "Build documentation" ON)

########################################################################################################################
########################################################################################################################
########################################################################################################################

message(STATUS "Use GoogleTest: ${USE_GOOLETEST}")
message(STATUS "Let CTest discover the tests: ${BUILD_TESTS}")
message(STATUS "Let CTest discover the complete tests: ${BUILD_COMPLETE_TESTS}")

message(STATUS "Enable parallel execution: ${ENABLE_PARALLEL}")
message(STATUS "Use OpenMP (if OFF, it will use TBB): ${USE_OPENMP}")


message(STATUS "Download and compile protobuf from git: " ${BUILD_PROTOBUF_SOURCE})

message(STATUS "Use pybind11 (downloaded automatically from github): " ${USE_PYBIND})

message(STATUS "Build documentation: ${BUILD_DOC}")


if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24.0")
    # Basically, files will be downloaded again based on download date/time, not extraction.
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

    if (USE_OPENMP)
        add_compile_definitions(PARALLEL_OPENMP)
    else ()
        add_compile_definitions(PARALLEL_TBB)
    endif ()

endif ()

set(
        INCLUDES
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


if (BUILD_DOC)
    include(doc/CMakeLists.txt)
endif ()


add_executable(network_butcher src/main.cpp ${BASE_SOURCE} ${PROTO_SRCS})
target_include_directories(network_butcher PUBLIC ${INCLUDES})
target_link_libraries(network_butcher ${RUN_LIBRARIES_LOCATION})


add_executable(main_kfinder mains/KFinder/main_kfinder.cpp ${BASE_SOURCE} ${PROTO_SRCS})
target_include_directories(main_kfinder PUBLIC ${INCLUDES} google_tests)
target_link_libraries(main_kfinder ${RUN_LIBRARIES_LOCATION})


add_executable(main_path_reconstruction mains/KFinder/main_path_reconstruction.cpp ${BASE_SOURCE} ${PROTO_SRCS})
target_include_directories(main_path_reconstruction PUBLIC ${INCLUDES} google_tests)
target_link_libraries(main_path_reconstruction ${RUN_LIBRARIES_LOCATION})


add_executable(main_IO_Manager mains/IO_Manager/main_IO_Manager.cpp ${BASE_SOURCE} ${PROTO_SRCS})
target_include_directories(main_IO_Manager PUBLIC ${INCLUDES})
target_link_libraries(main_IO_Manager ${RUN_LIBRARIES_LOCATION})


add_executable(main_Constrained_Builder mains/Butcher/main_Constrained_Builder.cpp ${BASE_SOURCE} ${PROTO_SRCS})
target_include_directories(main_Constrained_Builder PUBLIC ${INCLUDES})
target_link_libraries(main_Constrained_Builder ${RUN_LIBRARIES_LOCATION})


add_executable(main_synthetic_graph mains/KFinder/main_synthetic_graph.cpp ${BASE_SOURCE} ${PROTO_SRCS})
target_include_directories(main_synthetic_graph PUBLIC ${INCLUDES})
target_link_libraries(main_synthetic_graph ${RUN_LIBRARIES_LOCATION})


if (USE_GOOGLETEST)

    include(google_tests/CMakeLists.txt)

    enable_testing()


    add_executable(test_run ${TESTS_SOURCE} ${BASE_SOURCE})
    target_include_directories(test_run PUBLIC ${INCLUDES} google_tests)
    target_link_libraries(test_run ${TEST_LIBRARIES_LOCATION})


    add_executable(test_run_complete ${TESTS_SOURCE} ${COMPLETE_TESTS_SOURCE} ${BASE_SOURCE})
    target_include_directories(test_run_complete PUBLIC ${INCLUDES} google_tests)
    target_link_libraries(test_run_complete ${TEST_LIBRARIES_LOCATION})

    include(GoogleTest)

    if (DISCOVER_COMPLETE_TESTS)
        gtest_discover_tests(test_run_complete)
    elseif (DISCOVER_TESTS)
        gtest_discover_tests(test_run)
    endif ()

endif ()