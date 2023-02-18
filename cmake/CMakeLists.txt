include(FetchContent)

if (BUILD_PROTOBUF_SOURCE)
    set(Protobuf_USE_STATIC_LIBS ON)

    FetchContent_Declare(
            protobuf
            GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
            GIT_TAG        v3.17.3
            SOURCE_SUBDIR cmake
            OVERRIDE_FIND_PACKAGE
    )
    option(protobuf_BUILD_TESTS OFF)

    FetchContent_MakeAvailable(protobuf)

    list(APPEND TMP_LIBRARIES_LOCATION protobuf::libprotobuf)
else ()
    find_package(Protobuf REQUIRED)
    list(APPEND TMP_LIBRARIES_LOCATION ${Protobuf_LIBRARIES})
    message(STATUS "Found protobuf version ${Protobuf_VERSION}")
endif ()


if (USE_CPR_CPP)
    FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
            GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba) # The commit hash for v1.10.0
    FetchContent_MakeAvailable(cpr)

    list(APPEND TMP_LIBRARIES_LOCATION cpr::cpr)
endif()

# if(USE_PYBIND)
#FetchContent_Declare(
#        pybind11
#        GIT_REPOSITORY https://github.com/pybind/pybind11
#        GIT_TAG        v2.10.3
#)
#FetchContent_MakeAvailable(pybind11)
#include_directories(${pybind11_INCLUDE_DIRS})
# endif()


if (USE_YAML_CPP)

    if (BUILD_YAML_CPP_SOURCE)
        FetchContent_Declare(
                yaml-cpp
                GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
                GIT_TAG 0579ae3d976091d7d664aa9d2527e0d0cff25763
        )
        option(YAML_CPP_BUILD_TESTS OFF)
        FetchContent_MakeAvailable(yaml-cpp)

        list(APPEND TMP_LIBRARIES_LOCATION yaml-cpp)
    else ()
        find_package(yaml-cpp REQUIRED)

        include_directories(${YAML_CPP_INCLUDE_DIR})
        list(APPEND TMP_LIBRARIES_LOCATION ${YAML_CPP_LIBRARIES})
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
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/resnet18-v2-7-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/version-RFB-640-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/mobilenet_v2-inferred.onnx DESTINATION ${CMAKE_BINARY_DIR}/test_data/models)

    FILE(COPY ${CMAKE_SOURCE_DIR}/models/aMLLibrary_prediction_tegra.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/aMLLibrary_prediction_pi.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)

    FILE(COPY ${CMAKE_SOURCE_DIR}/models/official_operation_time.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_cloud.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_cloud_slow.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)
    FILE(COPY ${CMAKE_SOURCE_DIR}/models/operation_time_edge.csv DESTINATION ${CMAKE_BINARY_DIR}/test_data/weights)



    set(TEST_LIBRARIES_LOCATION ${TMP_LIBRARIES_LOCATION} gtest gtest_main PARENT_SCOPE)

endif ()

set(RUN_LIBRARIES_LOCATION ${TMP_LIBRARIES_LOCATION} PARENT_SCOPE)
