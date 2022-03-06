set(BASE_SOURCE
        src/Onnx_model/onnx.pb.h src/Onnx_model/onnx.pb.cc
        src/Network/Node.h src/Network/Graph.h
        src/Butcher.h
        src/Helpers/Computer/Computer_memory.h src/Helpers/Computer/Computer_memory.cpp src/Helpers/Computer/Hardware_specifications.h src/Helpers/Computer/Hardware_specifications.cpp src/Helpers/Computer/Computer_time.h src/Helpers/Computer/Computer_time.cpp
        src/Helpers/Types/Type_info.h src/Helpers/Types/Dense_tensor.cpp src/Helpers/Types/Dense_tensor.h src/Helpers/Types/Content.h
        src/Helpers/Utilities.h src/Helpers/Utilities.cpp src/Helpers/IO_Manager.h src/Helpers/IO_Manager.cpp
        src/Helpers/K-shortest_path/Shortest_path_finder.h src/Helpers/K-shortest_path/Path_info.h src/Helpers/K-shortest_path/Heap.h src/Helpers/K-shortest_path/Heap_eppstein.h src/Helpers/K-shortest_path/Heap_eppstein.cpp src/Helpers/K-shortest_path/KFinder.h src/Helpers/K-shortest_path/KEppstein.h src/Helpers/K-shortest_path/KEppstein_lazy.h
        src/Helpers/Traits/Graph_traits.h src/Helpers/Traits/Node_traits.h src/Helpers/Traits/Basic_traits.h src/Helpers/Traits/Type_info_traits.h src/Helpers/Traits/Hardware_traits.h
        src/Helpers/APCS/Factory.h src/Helpers/APCS/chrono.h src/Helpers/APCS/chrono.cpp
        PARENT_SCOPE)
add_subdirectory(Onnx_model)