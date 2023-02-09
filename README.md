# Network Butcher

### Requirements:

- cmake version 3.21 (or higher)
- g++ version 8 (or higher)
- (optional) doxygen

The remaining libraries are automatically installed by the program

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