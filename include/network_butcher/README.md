## include/network_butcher Directory Contents

This directory contains the source files of the program. The main directories are:

- APSC: This directory contains the header files of libraries taken from the APSC repository.
  For more details of its contents, referer to its [README](APSC/README.md) file
- Butcher: It contains the classes related to the butchering of a given DNN (Butcher and block graph building).
  For more details of its contents, referer to its [README](Butcher/README.md) file
- Computer: It contains the header file required to compute the memory usage (input and output tensors, as well as 
  parameters) of the layers of a given DNN. 
  All the functions are template functions.
- Extra: This directory contains some extra header files, that may be generated based on the CMake configuration
- IO_Interaction: It contains the header files used to interact with the storage (import and export of .onnx files and 
  weight import/generation). For more details of its contents, refer to its [README](IO_Interaction/README.md) file
- K-shortest_path: It contains the header files for the K-shortest path algorithm. Notice that there are two "final" 
  K-shortest path algorithms: Eppstein and Lazy Eppstein. A graph class may be used if it specializes Weighted_Graph. 
  For more details of its contents, referer to its [README](K-shortest_path/README.md) file
- Network: It contains the header files for the Network representation. MWGraph and WGraph are used to represent 
weighted directed graphs, while Graph represents a simple directed graph
- Traits: It contains some basic traits.
- Types: It contains the header files for the custom types used to import a .onnx file as well as to perform the butchering

The directory also contains:

- general_manager.h: The header file for General_Manager, the namespace containing all the functions used to perform 
the import, butchering and export of an .onnx model
- io_manager.h: The header file for IO_Manager, the namespace containing all the functions required by the program 
to interact with the IO
- network_butcher.h: By including this header file, a user can include ALL the required header files to perform any 
  operation with the library
- utilities.h: The header file for Utilities, the namespace containing all the extra helper functions