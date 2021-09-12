include(FetchContent)
set(FETCHCONTENT_QUIET ON)

if( ${BUILD_PROTOBUF_SOURCE} )
    #set(protobuf_BUILD_TESTS OFF)

    FetchContent_Declare(
            protobuf
            GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
            GIT_TAG        909a0f36a10075c4b4bc70fdee2c7e32dd612a72 # v3.17.3
            SOURCE_SUBDIR  cmake
    )
    set(protobuf_BUILD_TESTS OFF)
    FetchContent_MakeAvailable(protobuf)



else()
    find_package(Protobuf CONFIG REQUIRED)
endif()

if( NOT ${GTEST_FOUND})
    # Google tests
    add_subdirectory(google_tests)
endif()
