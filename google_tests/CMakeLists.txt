#set(TESTS_SOURCE
#
#        google_tests/Butcher_benchmark.cpp
#        PARENT_SCOPE)

set(
        TESTS_SOURCE
        google_tests/Helpers/Utilities.cpp
        google_tests/TestClass.h google_tests/TestClass.h
        google_tests/Helpers/Types/Dense_tensor.cpp google_tests/Helpers/Computer/Computer.cpp
        google_tests/Network/Node.cpp google_tests/Network/Graph.cpp
        google_tests/Helpers/K-shortest_path/Shortest_path_finder.cpp
        google_tests/Butcher.cpp
        PARENT_SCOPE
)