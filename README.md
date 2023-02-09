# network_butcher

### Requirements:

- cmake version 3.21 (or higher)
- g++
- (optional) doxygen

The remaining libraries are automatically installed by the program

### Installation:

1. Clone the repo
2. Change directory to the repository directory: 
```bash
cd path_to_repo
```

3. Create the build directory and move there:
```bash
mkdir build
cd build
```

4. Prepare the directory for the build:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -S .. -B . 
```

5. Build the .exe:
```bash
cmake --build . --target network_butcher
```

6. (Optional) Build the tests:
```bash
cmake --build . --target test_run
```
