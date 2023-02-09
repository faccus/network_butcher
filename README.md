# network_butcher

### Requirements:

- cmake version 3.21 (or higher)
- g++
- (optional) doxygen

The remaining libraries are automatically installed by the program

### Installation:

1. Clone the repo
2. Change directory to the repository directory: 
'''bash
cd path_to_repo
'''
3. Prepare the build (and install dependencies):
'''bash
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build 
'''
4. Build:
'''bash
cmake --build build --target network_butcher
'''
5. (Optional) Build tests:
'''bash
cmake --build build --target test_run
'''
