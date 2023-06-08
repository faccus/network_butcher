## Contents

This directory contains the source files of the program. The main directories are:

- APSC: This directory contains the source files of libraries taken from the APSC repository
- IO_Interaction: It contains the source files used to interact with the onnx files and the yaml files
- Types: It contains the source files for the custom types

This directory also contains:
- main.cpp: the main file of the network_butcher application
- utilities.cpp: the source file of the Utility functions
- io_manager.cpp: the Input/Output manager of the program. All the IO interactions that the program performs happen here
- general_manager.cpp: it contains the basic pipeline followed by the program to butcher a network
