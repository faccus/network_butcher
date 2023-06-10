## IO_Interaction Contents
It contains the header files used to interact with the storage (import and export of .onnx files and weight 
import/generation):
- Weight_Importer_Helpers/weight_importer.h contains the pure virtual class Weight_Importer, a simple class that represents 
  a weight importer. It exposes a single method, import_weights, that must be overridden by its children classes
- Weight_Importer_Helpers/weight_importer_utils.h contains some utility functions and structures that the various
  weight importers may use.
- Weight_Importer_Helpers/csv_weight_importer.h contains the template class Csv_Weight_Importer, that will be responsible
  for importing in the provided graph the weights read from one or more CSV files
- Weight_Importer_Helpers/block_aMLLibrary_weight_importer.h contains the class Block_aMLLibrary_Weight_Importer, the
  class that, interacting with onnx_tool and aMLLibrary through pybind, will generate and import the weights in the 
  provided block graph
- weight_importers.h is a simple header file that can be used to easily include the various weight importers in a file.


- onnx_importer_helpers.h contains all the helper functions used to convert an Onnx model to a graph
- onnx_model_reconstructor_helpers.h contains all the helper functions used to reconstruct, given a partitioning and
  the original ModelProto, the models associated with the various partitions