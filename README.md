# Network Butcher
The objective of this project is to develop a C++20 library (currently, only static) that can find, given a Deep Neural 
Network in Onnx format, the K optimal ways of partitioning it in order to minimize the overall execution time of the 
model itself on multiple devices.

The documentation can be manually generated by using Doxygen (as presented below) or by accessing 
it through [GitHub Pages](https://faccus.github.io/network_butcher/).

### Project structure

The project is structured into four main directories:

- include/network_butcher: It contains all the header files of the library. 
  A [README](include/network_butcher/README.md) file describing its contents is also available.
- src: It contains all the source files of the library. 
  Moreover, in the CMakeList file, all the external libraries used by the library are imported (except GoogleTest and
  Doxygen). 
  A [README](src/README.md) file describing its contents is also available.
- google_tests: It contains all the source files of the tests
- mains: It contains both a collection of main files to test the performances of the program and the basic main file
  that allows to perform the partitioning directly from command-line.
  A [README](mains/README.md) file describing its contents is also available.

The other directories are:

- dep: It contains the external libraries (currently, only aMLLibrary). They are stored as submodules.
- doc: It contains the CMakeLists.txt file with the options used to generate the documentation
- scripts: It contains some extra scripts to modify an .onnx file to add shapes to all the tensors (it must be executed
  for a new .onnx model, unless shapes were already inferred).
  Moreover, the notebooks used to create the plots of the measured performances are also present (as well as the actual
  results)
- test_data: It contains the files used during testing (models, sample configuration files and sample weight files).
  A [README](test_data/README.md) file describing its contents is also available.

To easily start reading the project, we think it's best to start from the General_Manager files.

### Requirements:

- CMake version 3.21 (or higher)
- g++ version 8 (or higher)

The remaining libraries (except Doxygen) can be automatically downloaded and (statically) linked by CMake. 
They are:

- [Protobuf v3.21.12](https://github.com/protocolbuffers/protobuf/tree/v21.12) (Required). 
  This library may be manually installed and linked against our library by checking the appropriate option in the 
  CMakeLists.txt file 
- [Google Tests v1.12.1](https://github.com/google/googletest/releases/tag/release-1.12.1) (Optional)
- [Doxygen v1.9.3 or higher](https://github.com/doxygen/doxygen/releases/tag/Release_1_9_3) (Optional)

The weight generation may be automatically performed if a compatible model is provided. 
This, however, adds some extra requirements:

- [PyBind v2.10.3](https://github.com/pybind/pybind11/releases/tag/v2.10.3).
  The library will be automatically downloaded and compiled by CMake. 
  However, it requires Python3 and pip3 to be installed.
- [aMLLibrary](https://github.com/brunoguindani/aMLLibrary/tree/933be8b094ca468d4813fe0c837fc6d46cc608d2) (Python github
  project). 
  A local version of the package is already available (as a submodule) in the dep directory. 
  Since the package is still work in progress, we chose a specific commit.
  For an explanation on why it is employed and a quick tutorial on its usage, please refer to the 
  [README](test_data/aMLLibrary_data/README.md) file.
  To install all of its package dependencies, simply call in our repository root directory:
```bash
python3 -m pip install -r dep/aMLLibrary/requirements.txt
```
- [Onnx-Tool v0.6.1](https://github.com/ThanatosShinji/onnx-tool) (Python Package). To install it, simply call:
```bash
python3 -m pip install onnx-tool==0.6.1
```

The CMakeLists.txt file exposes some options that may be changed to avoid the usage of some of the previously cited
libraries.

Parallelism can be enabled by checking the proper option in the CMakeLists.txt file. 
Parallelism may be used with either:

- [OpenMP v4.5](https://www.openmp.org/). 
  Since we do not fix the number of parallel threads that the library can use directly in the code, we advise setting 
  the environment variable OMP_NUM_THREADS to the desired number of threads.
- [Intel TBB v2021.5.0](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html). Notice that both
  TBB and oneTBB are compatible. However, the former is easier to install through a package manager (for instance, in
  Ubuntu, the package is available as libtbb-dev), while the latter must be compiled from source. Since this library is
  meant to be used by the C++ standard library to employ parallelism and, thus, no newer functionality is employed, we
  advise to use TBB.

We advise, if undecided on which library to use, to employ OpenMP, since it is easier to install and use.

### Requirements Quick Install (Linux only)
We here report a small collection of simple bash commands to install the required libraries on a Linux system:
- Compiler, CMake and Doxygen (with its extra packages):
```bash
echo 'Initial package refresh:'
sudo apt-get update
echo 'Installing the compiler and CMake:'
sudo apt-get install build-essential cmake -y
echo 'Installing Doxygen (and Graphviz, dia and mscgen):'
sudo apt-get install doxygen graphviz dia mscgen -y
```
- Python, pip, aMLLibrary requirements and onnx-tool:
```bash
echo 'Installing python and pip:'
sudo apt-get install python3-dev python3-pip --no-install-recommends -y
echo 'Installing aMLLibrary requirements:'
python3 -m pip install -r requirements.txt
echo 'Installing onnx-tool:'
python3 -m pip install onnx-tool==0.6.1
```
- OpenMP and TBB:
```bash
echo 'Installing OpenMP:'
sudo apt-get install libomp-14-dev -y
echo 'Installing TBB:'
sudo apt-get install libtbb-dev -y
```

### Docker
A Dockerfile is available to speed up the initial setup.
The file contains all the required dependencies of the project

To build the image, after having cloned the project, simply run from the project directory:
```bash
docker build -t network_butcher .
```

To run the container and enter the bash shell, run:
```bash
docker run --name nn --rm -v $(pwd):/network_butcher -it network_butcher
```

### Project setup:

1. Clone the repository:

```bash
git clone https://github.com/faccus/network_butcher --recurse-submodules
```

or

```bash
git clone git@github.com:faccus/network_butcher.git --recurse-submodules
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

4. Build the executable:

```bash
cmake --build . --target main
```

5. Run the program:

```bash
./main
```

### Profiling mains

To properly verify the performances of the associated library, we decided to make available a collection of main files
to properly measure the time required to perform each phase of the "butchering".
They can be easily compiled by calling after step 3:

```bash
cmake --build . --target main_Constrained_Builder
cmake --build . --target main_IO_Manager
cmake --build . --target main_kfinder
cmake --build . --target main_path_reconstruction
cmake --build . --target main_synthetic_graph
```

### Doxygen
To run doxygen and generate the documentation, you can just call from the build directory:
```bash
cmake --build . --target doxygen
```


### Tests

Optionally, the tests can be built by using the following command:

```bash
cmake --build . --target test_run
```

and executed by:

```bash
./test_run
```

By changing the target to 'test_run_complete' a longer collection of tests will be executed.
This is useful to check the correctness of the program, but it may take a while to complete.