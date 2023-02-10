# Network Butcher

### Requirements:

- cmake version 3.21 (or higher)
- g++ version 8 (or higher)

The remaining libraries (with the exception of Doxygen) can automatically be downloaded and (statically) linked by the program. They may be installed manually:
- [Protobuf v3.17.3](https://github.com/protocolbuffers/protobuf/releases/tag/v3.17.3) (Required)
- [Google Tests v1.12.1](https://github.com/google/googletest/releases/tag/release-1.12.1) (Optional)
- [Yaml-Cpp v0.7.0](https://github.com/jbeder/yaml-cpp/releases/tag/yaml-cpp-0.7.0) (Optional)
- [Doxygen v1.9.3](https://github.com/doxygen/doxygen/releases/tag/Release_1_9_3) (Optional)

To (dynamically) link the installed libraries, check the CMakeLists file in the root directory of the repository.

### Installation:

1. Clone the repository:
```bash
git clone https://github.com/faccus/network_butcher
```
or
```bash
git clone git@github.com:faccus/network_butcher.git
```
2. Create the build directory and move there:
```bash
cd network_butcher
mkdir build
cd build
```
3. Prepare the directory for the build:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -S .. -B . 
```
4. Build the .exe:
```bash
cmake --build . --target network_butcher
```
5. Run the program:
```bash
./network_butcher
```

Optionally, the tests can be built by using the following command:
```bash
cmake --build . --target test_run
```
and executed by:
```bash
./test_run
```

### Project structure
The project is structured into three main directories:
- include: It contains all the header files
- src: It contains all the source files
- google_tests: It contains all the source files of the tests

The other directories are:
- models: It contains the files used during testing (models, sample configuration files and sample weight files)
- scripts: It contains some extra scripts to modify an .onnx file to add shapes to all the tensors (it must be executed for a new .onnx model, unless shapes were already inferred)
- doc: It contains the CMakeLists.txt file with the options used to generate the documentation
- cmake: It contains the CMakeLists.txt file to either download and compile external libraries or to dynamically link them
