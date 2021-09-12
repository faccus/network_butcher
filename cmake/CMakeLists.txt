include(FetchContent)
set(FETCHCONTENT_QUIET ON)

if( ${BUILD_PROTOBUF_SOURCE} )
    set ( Protobuf_USE_STATIC_LIBS ON )
    #set(protobuf_BUILD_TESTS OFF)

    FetchContent_Declare(
            protobuf
            URL https://github.com/protocolbuffers/protobuf/releases/download/v3.17.3/protobuf-cpp-3.17.3.zip
            SOURCE_SUBDIR  cmake
    )
    option(protobuf_BUILD_TESTS OFF)
    FetchContent_MakeAvailable(protobuf)

    include_directories(${protobuf_SOURCE_DIR}/src/google)
else()
    find_package(Protobuf REQUIRED)
endif()

message(STATUS "Protobuf version ${Protobuf_VERSION}")

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
FetchContent_MakeAvailable(googletest)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
