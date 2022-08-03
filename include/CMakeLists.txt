set(BASE_HEADER
        include/Network/Node.h include/Network/Graph.h include/Network/WGraph.h include/Network/MWGraph.h
        include/Butcher.h include/Types/Paths.h
        include/Utilities.h include/General_Manager.h include/Types/Parameters.h include/IO_Manager.h
        include/Types/Type_info.h include/Types/Dense_tensor.h include/Types/Content.h include/Types/DynamicType.h
        include/Traits/Graph_traits.h include/Traits/Node_traits.h include/Traits/Basic_traits.h
        include/Traits/Type_info_traits.h include/Traits/Hardware_traits.h include/Traits/Heap_traits.h
        include/K-shortest_path/Shortest_path_finder.h include/K-shortest_path/Path_info.h include/K-shortest_path/Heap.h
        include/K-shortest_path/Heap_eppstein.h include/K-shortest_path/KFinder.h
        include/K-shortest_path/KEppstein.h include/K-shortest_path/KEppstein_lazy.h
        include/APSC/Factory.h include/APSC/chrono.h include/APSC/GetPot
        include/Computer/Computer_memory.h include/Computer/Hardware_specifications.h include/Computer/Computer_time.h
        PARENT_SCOPE)