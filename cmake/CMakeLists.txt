include(FetchContent)

if( BUILD_PROTOBUF_SOURCE )
    set ( Protobuf_USE_STATIC_LIBS ON )

    FetchContent_Declare(
            protobuf
            URL https://github.com/protocolbuffers/protobuf/releases/download/v3.17.3/protobuf-cpp-3.17.3.zip
            SOURCE_SUBDIR  cmake
    )
    option(protobuf_BUILD_TESTS OFF)
    option(protobuf_BUILD_PROTOC_BINARIES OFF)
    FetchContent_MakeAvailable(protobuf)

    set(PROTOBUF_LIBRARY_LOCATION protobuf::libprotobuf)
else()
    find_package(Protobuf REQUIRED)
    set(PROTOBUF_LIBRARY_LOCATION ${Protobuf_LIBRARIES})
    message(STATUS "Found protobuf version ${Protobuf_VERSION}")
endif()

if ( BUILD_TESTS )
    # Add google test library from git
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
    )
    FetchContent_MakeAvailable(googletest)

    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    # Add model for tests
    FILE(COPY ${CMAKE_SOURCE_DIR}/src/Models/resnet18-v2-7-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR})
endif()

set(TMP_LIBRARIES_LOCATION ${PROTOBUF_LIBRARY_LOCATION})
set(RUN_LIBRARIES_LOCATION ${TMP_LIBRARIES_LOCATION} PARENT_SCOPE)
set(TEST_LIBRARIES_LOCATION ${TMP_LIBRARIES_LOCATION} gtest gtest_main PARENT_SCOPE)