//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_PARAMETERS_H
#define NETWORK_BUTCHER_PARAMETERS_H

#include "Graph_traits.h"

namespace network_butcher
{
  namespace parameters
  {
    /// Enumerator for the weight import modes
    enum Weight_Import_Mode
    {
      single_direct_read,
      multiple_direct_read,
      aMLLibrary_inference_original,
      aMLLibrary_inference_block
    };

    /// Collection of parameters for a device
    struct Device
    {
      /// Device id
      std::size_t id;

      /// Device name
      std::string name;

      /// Maximum memory capacity (in bytes)
      memory_type maximum_memory;

      /// The .csv file of weights
      std::string weights_path;

      /// The column of the .csv file containing the weights
      std::string relevant_entry;
    };

    /// Enumerator for the different KSP methods
    enum KSP_Method
    {
      Eppstein,
      Lazy_Eppstein
    };

    /// The memory constraints possibilities
    enum Memory_Constraint_Type
    {
      None,
      Max,
      Preload_Parameters
    };

    /// The collection of parameters
    struct Parameters
    {
      /// The model name
      std::string model_name;

      /// The model path
      std::string model_path;

      /// The export directory
      std::string export_directory;

      /// The temporary directory
      std::string temporary_directory;

      /// The (absolute) path to the onnx_tool package (if not default)
      std::vector<std::string> extra_packages_location;

      /// The number of paths to return
      std::size_t K;

      /// The KSP method
      KSP_Method method;

      /// Starting device
      std::size_t starting_device_id;

      /// End device
      std::size_t ending_device_id;

      /// Are backward collection allowed? (i.e. can device j send data to device k with j>k?)
      bool backward_connections_allowed;

      /// Mode for weight import
      Weight_Import_Mode weight_import_mode;

      /// The variables of the .csv file that aMLLibrary should
      std::vector<std::string> aMLLibrary_inference_variables;

      /// The features of the .csv file to feed to aMLLibrary
      std::vector<std::string> aMLLibrary_csv_features;

      /// The path of the .csv file that stores all the weights
      std::string single_weight_import_path;

      /// The columns of the .csv file that store the weights
      std::vector<std::string> single_csv_columns_weights;

      /// The separator
      char separator;

      /// Do we have to check for memory constraints?
      bool memory_constraint;

      /// The type of memory constraint
      Memory_Constraint_Type memory_constraint_type;

      /// The collection of devices
      std::vector<Device> devices;

      /// The bandwidth information between the different devices (Mbps - s)
      std::map<std::pair<std::size_t, std::size_t>, std::pair<bandwidth_type, bandwidth_type>> bandwidth;
    };
  } // namespace parameters

} // namespace network_butcher


#endif // NETWORK_BUTCHER_PARAMETERS_H
