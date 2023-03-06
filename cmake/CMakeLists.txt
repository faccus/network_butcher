include(FetchContent)

if (BUILD_PROTOBUF_SOURCE)
    set(Protobuf_USE_STATIC_LIBS ON)

    FetchContent_Declare(
            protobuf
            GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
            GIT_TAG        v3.21.12
    )
    option(protobuf_BUILD_TESTS OFF)

    FetchContent_MakeAvailable(protobuf)

    list(APPEND RUN_LIBRARIES_LOCATION protobuf::libprotobuf)

    list(APPEND EXTRA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/onnx_proto)
    list(APPEND BASE_SOURCE ${CMAKE_SOURCE_DIR}/onnx_proto/onnx.pb.cc)

else ()
    find_package(Protobuf REQUIRED)
    list(APPEND RUN_LIBRARIES_LOCATION ${Protobuf_LIBRARIES})

    if(EXISTS ${CMAKE_SOURCE_DIR}/onnx_proto/onnx)
        file(REMOVE ${CMAKE_SOURCE_DIR}/onnx_proto/onnx)
    endif()

    file(DOWNLOAD https://raw.githubusercontent.com/onnx/onnx/main/onnx/onnx.proto3 ${CMAKE_SOURCE_DIR}/onnx_proto/onnx)

    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/onnx_proto/onnx)

    list(APPEND EXTRA_INCLUDE_DIR ${CMAKE_BINARY_DIR})
    list(APPEND BASE_SOURCE ${CMAKE_BINARY_DIR}/onnx.pb.cc)
endif ()


if (USE_CPR_CPP)
    FetchContent_Declare(
            cpr
            GIT_REPOSITORY https://github.com/libcpr/cpr.git
            GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba
    ) # The commit hash for v1.10.0
    FetchContent_MakeAvailable(cpr)

    list(APPEND RUN_LIBRARIES_LOCATION cpr::cpr)
endif()

if(USE_PYBIND)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.10.3
        OVERRIDE_FIND_PACKAGE
    )
    find_package(pybind11 REQUIRED)

    #FetchContent_MakeAvailable(pybind11)
    include_directories(${pybind11_INCLUDE_DIRS})

    list(APPEND RUN_LIBRARIES_LOCATION pybind11::embed pybind11::pybind11)
endif()


if (USE_YAML_CPP)
    if (BUILD_YAML_CPP_SOURCE)
        FetchContent_Declare(
                yaml-cpp
                GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
                GIT_TAG 0579ae3d976091d7d664aa9d2527e0d0cff25763
        )
        option(YAML_CPP_BUILD_TESTS OFF)
        FetchContent_MakeAvailable(yaml-cpp)

        list(APPEND RUN_LIBRARIES_LOCATION yaml-cpp)
    else ()
        find_package(yaml-cpp REQUIRED)

        include_directories(${YAML_CPP_INCLUDE_DIR})
        list(APPEND RUN_LIBRARIES_LOCATION ${YAML_CPP_LIBRARIES})
    endif ()
endif ()


if (BUILD_TESTS)
    # Add google test library from git
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})


    set(TEST_LIBRARIES_LOCATION ${RUN_LIBRARIES_LOCATION} gtest gtest_main)
endif ()
