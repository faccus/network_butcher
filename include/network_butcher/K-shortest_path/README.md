## K-shortest_path Contents
This directory contains all the classes, structures and functions required to apply on a graph the K shortest path 
algorithm.

We will divide this file in three sections:

### Basic Data structures
The files that provide data structures used in the other parts of this directory are:
- weighted_graph.h . It contains the template proxy/interface class Weighted_Graph, that MUST be specialized by all
  graph types that will apply the algorithms presented later on
- path_info.h contains Templated_Path_Info, a structure that stores an (explicit) path of the graph and its length
- crtp_greater.h contains Crtp_Greater, a simple structure that, using the CRTP pattern, will implement the greater 
  operator using the < operator.
- heap_eppstein.h contains:
    - Heap_Node represents a node in a min heap with up to Max_Children children per node.
    - H_out_Type represents an H_out, a 2-heap with the extra restriction that its root node has a single child
    - H_g_Type represents an H_g. Due to the specifics of the Eppstein and Lazy Eppstein algorithm, it will manually
      construct the underlying heap and will implement the "persistent" merge with other H_g_Type
    - Templated_Edge_Info represents a sidetrack edge, a structure containing an edge and its sidetrack weight

### Utilities 
The files that provide utility functions for the K shortest path methods are:
- shortest_path_finder.h contains the function required to perform the Dijstrika algorithm and to construct the single
  destination shortest path tree.
- ksp_method.h contains a simple enumerator for the various implemented K shortest path methods.

### K shortest path algorithms
The files that contain the classes used to find the K shortest paths are:
- kfinder.h contains KFinder, a pure virtual template class used as a basis for any K shortest path algorithm. It exposes
  a 'compute' method that must be specialized by its children classes.
- kfinder_factory.h contains a simple factory class, used to easily generate the various KFinder classes
- basic_keppstein.h contains Basic_KEppstein, a pure virtual template class, child of KFinder, that provides all the common 
  methods used by both the Eppstein and Lazy Eppstein algorithms
- keppstein.h contains KFinder_Eppstein, the template class, child of Basic_KEppstein, that implements the Eppstein algorithm
- keppstein_lazy.h contains KFinder_Lazy_Eppstein, the template class, child of Basic_KEppstein, that implements the Lazy Eppstein algorithm