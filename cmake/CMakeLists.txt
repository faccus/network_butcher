include(FetchContent)

if (BUILD_PROTOBUF_SOURCE)
    set(Protobuf_USE_STATIC_LIBS ON)

    FetchContent_Declare(
            protobuf
            URL https://github.com/protocolbuffers/protobuf/releases/download/v3.17.3/protobuf-cpp-3.17.3.zip
            SOURCE_SUBDIR cmake
    )
    option(protobuf_BUILD_TESTS OFF)

    FetchContent_MakeAvailable(protobuf)

    set(PROTOBUF_LIBRARY_LOCATION protobuf::libprotobuf)
else ()
    find_package(Protobuf REQUIRED)
    set(PROTOBUF_LIBRARY_LOCATION ${Protobuf_LIBRARIES})
    message(STATUS "Found protobuf version ${Protobuf_VERSION}")
endif ()

if (USE_YAML_CPP)

    if (BUILD_YAML_CPP_SOURCE)
        FetchContent_Declare(
                yaml-cpp
                GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
                GIT_TAG 0579ae3d976091d7d664aa9d2527e0d0cff25763
        )
        set(YAML_CPP_BUILD_TESTS OFF)
        FetchContent_MakeAvailable(yaml-cpp)
        message(STATUS "Downloaded yaml-cpp version ${yaml-cpp_VERSION}")

        set(YAML_CPP_LIBRARY_LOCATION yaml-cpp)
    else ()
        find_package(yaml-cpp REQUIRED)

        include_directories(${YAML_CPP_INCLUDE_DIR})
        set(YAML_CPP_LIBRARY_LOCATION ${YAML_CPP_LIBRARIES})
        message(STATUS "Found yaml-cpp version ${yaml-cpp_VERSION}")
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

    # Add model for tests
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/resnet18-v2-7-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/version-RFB-640-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/mobilenet_v2-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR})

    FILE(COPY ${CMAKE_SOURCE_DIR}/models/aMLLibrary_prediction_tegra.csv DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/aMLLibrary_prediction_pi.csv DESTINATION ${CMAKE_BINARY_DIR})

    FILE(COPY ${CMAKE_SOURCE_DIR}/models/official_operation_time.csv DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_cloud.csv DESTINATION ${CMAKE_BINARY_DIR})
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_edge.csv DESTINATION ${CMAKE_BINARY_DIR})
endif ()


set(TMP_LIBRARIES_LOCATION ${PROTOBUF_LIBRARY_LOCATION} ${YAML_CPP_LIBRARY_LOCATION})
set(RUN_LIBRARIES_LOCATION ${TMP_LIBRARIES_LOCATION} PARENT_SCOPE)
set(TEST_LIBRARIES_LOCATION ${TMP_LIBRARIES_LOCATION} gtest gtest_main PARENT_SCOPE)