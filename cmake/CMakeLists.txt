include(FetchContent)

if( BUILD_PROTOBUF_SOURCE )
    set ( Protobuf_USE_STATIC_LIBS ON )

    FetchContent_Declare(
            Protobuf
            URL https://github.com/protocolbuffers/protobuf/releases/download/v3.20.1/protobuf-cpp-3.20.1.zip
            #SOURCE_SUBDIR cmake
            OVERRIDE_FIND_PACKAGE
    )
    option(protobuf_BUILD_TESTS OFF)
    option(protobuf_BUILD_PROTOC_BINARIES OFF)
endif()

find_package(Protobuf REQUIRED)
set(PROTOBUF_LIBRARY_LOCATION ${Protobuf_LIBRARIES})

message(STATUS "Found protobuf version ${Protobuf_VERSION}")

set(PROTOBUF_LIBRARY_LOCATION ${Protobuf_LIBRARY})
include_directories(${Protobuf_INCLUDE_DIR})


if( USE_YAML_CPP )

    find_package(yaml-cpp REQUIRED)
    set(YAML_CPP_LIBRARY_LOCATION ${YAML_CPP_LIBRARIES})
    message(STATUS "Found protobuf version ${Protobuf_VERSION}")

    include_directories(${YAML_CPP_INCLUDE_DIRS})
endif()


if ( BUILD_TESTS )
    # Add google test library from git
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)

    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    # Add model for tests
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/resnet18-v2-7-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/version-RFB-640-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/mobilenet_v2-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR})

    FILE(COPY ${CMAKE_SOURCE_DIR}/models/aMLLibrary_prediction_tegra.csv DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/aMLLibrary_prediction_pi.csv DESTINATION ${CMAKE_BINARY_DIR})

    FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_cloud.csv DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_edge.csv DESTINATION ${CMAKE_BINARY_DIR})
endif ()

#find_package(Boost REQUIRED COMPONENTS)
#include_directories(${Boost_INCLUDE_DIRS})


set(TMP_LIBRARIES_LOCATION ${PROTOBUF_LIBRARY_LOCATION} ${YAML_CPP_LIBRARY_LOCATION})# ${Boost_LIBRARIES})

set(RUN_LIBRARIES_LOCATION ${TMP_LIBRARIES_LOCATION} PARENT_SCOPE)
set(TEST_LIBRARIES_LOCATION ${TMP_LIBRARIES_LOCATION} gtest gtest_main PARENT_SCOPE)