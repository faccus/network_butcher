set(
        TESTS_SOURCE
        google_tests/TestClass.h google_tests/TestClass.h
        google_tests/Types/Dense_tensor.cpp
        google_tests/Utilities.cpp
        google_tests/Onnx_interaction/Onnx_importer_helpers.cpp
        google_tests/Network/Node.cpp
        google_tests/Computer/Computer.cpp
        google_tests/Network/Graph.cpp
        google_tests/IO_Manager.cpp
        google_tests/K-shortest_path/Shortest_path_finder.cpp
        google_tests/Butcher.cpp
        PARENT_SCOPE
)

set(
        BENCHMARK_TESTS_SOURCE
        google_tests/General_Manager.cpp
        google_tests/Butcher_benchmark.cpp
        PARENT_SCOPE
)