## Butcher Contents
Here are collected the header files associated to the partitioning of a graph:
- butcher.h: It contains the template class Butcher, the class that, given a graph, will call all the required methods 
  to construct the block graph, perform the K shortest path algorithm and obtain the final partitionings.
- constrained_block_graph_builder.h contains the template class Constrained_Block_Graph_Builder, the builder class that 
  will construct from a given graph the associated block graph and, if the user specified it by calling the appropriate
  functions, it will apply the operation and transmission weights, as well as the specified constraints (that will act
  on the final constructed block graph)
- graph_constraint.h contains the abstract class Graph_Constraint, used to represent a generic constraint to be applied
  on the block graph. It also contains a child template class, called Memory_Constraint, that will add a constraint on
  the overall memory usage of a partitioning.
- path_converter.h contains the class Path_Converter, that will be responsible for converting a path on the block graph
  to an actual partitioning of the original graph