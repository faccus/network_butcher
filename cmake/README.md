## cmake Contents
In the file CMakelists.txt, all the external libraries are imported and made available to be linked in the "main" CMakeList file.
The imported libraries are:
- Protobuf: It will be either downloaded from github or imported from the system. If it is imported from the system,
  the user must also make available the prtoobuf compiler 'protoc' in the system. Moreover, if the library is imported,
  the newest onnx.proto3 scheme will be downloaded from github, compiled through protoc and made available to the project.
- Pybind11: If the corresponding option is selected in the main CMakeList file, the library will be downloaded from github.
  We decided to NOT allow the user to import the library directly from their system because it's the procedure that the
  developers of Pybind11 suggest.
  It is employed to use onnx_tool and aMLLibrary during the weight import phase.
- TBB or OpenMP: If parallelism is enabled, the user can choose between TBB and OpenMP. If the corresponding option is selected
  in the main CMakeList file, the library will be imported from the system.
- GoogleTest: If the corresponding option is selected in the main CMakeList file, the library will be downloaded from github.
  We decided to NOT allow the user to import the library directly from their system because it's the procedure that the
  developers of GoogleTest suggest in the documentation.
  It is employed to manage the tests of the project.
- Doxygen: If the corresponding option is selected in the main CMakeList file, the library will be imported from the system.
  It is employed to generate the documentation of the project.