set(BASE_HEADER
        include/Network/Node.h include/Network/Graph.h include/Network/WGraph.h include/Network/MWGraph.h
        include/Butcher.h include/Helpers/Types/Paths.h
        include/Helpers/Utilities.h include/Helpers/General_Manager.h include/Helpers/Types/Parameters.h include/Helpers/IO_Manager.h
        include/Helpers/Types/Type_info.h include/Helpers/Types/Dense_tensor.h include/Helpers/Types/Content.h include/Helpers/Types/DynamicType.h
        include/Helpers/Traits/Graph_traits.h include/Helpers/Traits/Node_traits.h include/Helpers/Traits/Basic_traits.h
        include/Helpers/Traits/Type_info_traits.h include/Helpers/Traits/Hardware_traits.h include/Helpers/Traits/Heap_traits.h
        include/Helpers/K-shortest_path/Shortest_path_finder.h include/Helpers/K-shortest_path/Path_info.h include/Helpers/K-shortest_path/Heap.h
        include/Helpers/K-shortest_path/Heap_eppstein.h include/Helpers/K-shortest_path/KFinder.h
        include/Helpers/K-shortest_path/KEppstein.h include/Helpers/K-shortest_path/KEppstein_lazy.h
        include/Helpers/APSC/Factory.h include/Helpers/APSC/chrono.h include/Helpers/APSC/GetPot
        include/Helpers/Computer/Computer_memory.h include/Helpers/Computer/Hardware_specifications.h include/Helpers/Computer/Computer_time.h
        PARENT_SCOPE)