include(FetchContent)

if (BUILD_PROTOBUF_SOURCE)
    find_package(Protobuf 3.21.12 EXACT)

    if ( Protobuf_FOUND )
        message("We found a compatible version of Protobuf! Thus, we will use it instead of downloading it from Github")
    else ()
        set(Protobuf_USE_STATIC_LIBS ON)

        FetchContent_Declare(
                protobuf
                GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
                GIT_TAG v3.21.12
        )
        option(protobuf_BUILD_TESTS OFF)
        FetchContent_MakeAvailable(protobuf)
    endif ()
else ()
    find_package(Protobuf 3.21.12 REQUIRED EXACT)
endif ()

list(APPEND RUN_LIBRARIES_LOCATION protobuf::libprotobuf)
list(APPEND HEADERS_LIBRARIES_LOCATION ${Protobuf_INCLUDE_DIRS})


if (USE_PYBIND)
    FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG v2.10.3
    )
    FetchContent_MakeAvailable(pybind11)

    if(EXISTS "${CMAKE_SOURCE_DIR}/include/network_butcher/Extra/CMake_directives.h")
        file(REMOVE ${CMAKE_SOURCE_DIR}/include/network_butcher/Extra/CMake_directives.h)
    endif ()

    configure_file(${CMAKE_SOURCE_DIR}/include/network_butcher/Extra/CMake_directives.h.in
                   ${CMAKE_SOURCE_DIR}/include/network_butcher/Extra/CMake_directives.h)

    if (EXISTS "${CMAKE_SOURCE_DIR}/include/network_butcher/Extra/platform_specific_config.h.in")
        file(REMOVE ${CMAKE_SOURCE_DIR}/include/network_butcher/Extra/platform_specific_config.h)

        configure_file(${CMAKE_SOURCE_DIR}/include/network_butcher/Extra/platform_specific_config.h.in
                       ${CMAKE_SOURCE_DIR}/include/network_butcher/Extra/platform_specific_config.h)
        add_compile_definitions(PLATFORM_SPECIFIC_CONFIG)
    endif ()

    list(APPEND EXTRA_INCLUDE_DIR ${CMAKE_BINARY_DIR})

    list(APPEND RUN_LIBRARIES_LOCATION pybind11::embed pybind11::pybind11)
    list(APPEND HEADERS_LIBRARIES_LOCATION ${pybind11_INCLUDE_DIRS})

    add_compile_definitions(NETWORK_BUTCHER_PYBIND_ACTIVE)
endif ()

if (ENABLE_NETWORK_BUTCHER_PARALLEL)
    add_compile_definitions(NETWORK_BUTCHER_PARALLEL)

    if (USE_OPENMP)
        find_package(OpenMP REQUIRED)
        list(APPEND RUN_LIBRARIES_LOCATION OpenMP::OpenMP_CXX)
        add_compile_definitions(NETWORK_BUTCHER_PARALLEL_OPENMP)

        list(APPEND HEADERS_LIBRARIES_LOCATION ${OpenMP_CXX_INCLUDE_DIRS})
    else ()
        find_package(TBB REQUIRED)
        list(APPEND RUN_LIBRARIES_LOCATION TBB::tbb)
        add_compile_definitions(NETWORK_BUTCHER_PARALLEL_TBB)
    endif ()
endif ()



set(BASE_SOURCE
        APSC/chrono.cpp
        IO_Interaction/onnx_importer_helpers.cpp
        IO_Interaction/onnx_model_reconstructor_helpers.cpp
        IO_Interaction/Weight_Importer_Helpers/weight_importer_utils.cpp
        IO_Interaction/Weight_Importer_Helpers/csv_weight_importer.cpp
        IO_Interaction/Weight_Importer_Helpers/block_aMLLibrary_weight_importer.cpp
        onnx_proto/onnx.pb.cc
        Types/dense_tensor.cpp
        general_manager.cpp
        io_manager.cpp
        utilities.cpp)

set(APPEND
        INCLUDES
        ${PROJECT_SOURCE_DIR}/include/network_butcher
        ${PROJECT_SOURCE_DIR}/include/network_butcher/APSC
        ${PROJECT_SOURCE_DIR}/include/network_butcher/Computer
        ${PROJECT_SOURCE_DIR}/include/network_butcher/Butcher
        ${PROJECT_SOURCE_DIR}/include/network_butcher/IO_Interaction
        ${PROJECT_SOURCE_DIR}/include/network_butcher/IO_Interaction/Weight_Importer_Helpers
        ${PROJECT_SOURCE_DIR}/include/network_butcher/K-shortest_path
        ${PROJECT_SOURCE_DIR}/include/network_butcher/Network
        ${PROJECT_SOURCE_DIR}/include/network_butcher/Traits
        ${PROJECT_SOURCE_DIR}/include/network_butcher/onnx_proto
        ${PROJECT_SOURCE_DIR}/include/network_butcher/Types
        ${EXTRA_INCLUDE_DIR}
)



# Make an automatic library - will be static or dynamic based on user setting
add_library(network_butcher ${BASE_SOURCE} ${INCLUDES} ${HEADERS_LIBRARIES_LOCATION})

# We need this directory, and users of our library will need it too
target_include_directories(network_butcher PUBLIC ../include ${HEADERS_LIBRARIES_LOCATION})

# This depends on (header only) boost
target_link_libraries(network_butcher PUBLIC ${RUN_LIBRARIES_LOCATION})

# All users of this library will need at least C++20
target_compile_features(network_butcher PUBLIC cxx_std_20)



if (USE_PYBIND)
    target_compile_definitions(network_butcher PUBLIC NETWORK_BUTCHER_PYBIND_ACTIVE)
endif ()

if( ENABLE_NETWORK_BUTCHER_PARALLEL )
    target_compile_definitions(network_butcher PUBLIC NETWORK_BUTCHER_PARALLEL)

    if( USE_OPENMP )
        target_compile_definitions(network_butcher PUBLIC NETWORK_BUTCHER_PARALLEL_OPENMP)
    else ()
        target_compile_definitions(network_butcher PUBLIC NETWORK_BUTCHER_PARALLEL_TBB)
    endif ()
endif ()


# IDEs should put the headers in a nice place
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${INCLUDES})