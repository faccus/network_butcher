## Contents

This directory contains the source files of the program. The main directories are:

- APSC: This directory contains the header files of libraries taken from the APSC repository
- Computer (WIP): It contains the header files required to compute the FLOPS, MACS and the memory usage 
(input and output tensors, as well as parameters) of the operations of a given NN. Almost all the functions are 
template functions.
- IO_Interaction: It contains the header files used to interact with the onnx files and the yaml files
- K-shortest_path: It contains the header files for the K-shortest path algorithm. All the classes introduced are 
template casses (an arbitrary Graph class may be used. I will probably modify the inputs so that the template class must implement a specific set of operations)
- Network: It contains the header files for the Network representation. MWGraph and WGraph are used to represent 
weighted directed graphs, while Graph represents a simple directed graph
- Traits (WIP): It contains some basic traits. I will probably change them when I finish to implement everything else 
- Types: It contains the header files for the custom types used to import a .onnx file as well as to perform the butchering

The directory also contains:

- Butcher.h: The header file for Butcher, the template class that performs the "butchering" of a given Graph
- General_Manager.h: The header file for General_Manager, the namespace containing all the functions used to perform 
the import, butchering and export of an .onnx model
- IO_Manager.h: The header file for IO_Manager, the namespace containing all the functions required by the program 
to interact with the IO
- Utilities.h: The header file for Utilities, the namespace containing all the extra helper functions