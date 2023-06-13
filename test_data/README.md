## test_data Directory Contents
This directory contains the files used during the tests of the project.
In particular, they are divided into five directories:
- aMLLibrary_data contains all the files required to train a regression model. 
It contains some example configuration files, some sample input files (for the training) and some example pre-trained
models
- configs contains the configuration files used to boot tests.
In the directory, we have also provided a template configuration file that can be edited to run network_butcher
- kfinder contains the sample graphs and the corresponding (correct) outputs of the K shortest paths algorithms.
They were both generated using [library checker problems](https://github.com/yosupo06/library-checker-problems) project,
available on github
- models contains some sample Onnx models
- weights contains some sample weight files (in CSV format), used during the boot tests