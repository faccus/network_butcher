//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_PARAMETERS_H
#define NETWORK_BUTCHER_PARAMETERS_H

#include "Graph_traits.h"

namespace network_butcher_parameters
{
  /// Enumerator for the weight import modes
  enum Weight_Import_Mode
  {
    aMLLibrary_direct_read,
    operation_time,
    multi_operation_time,
    official_operation_time,
    aMLLibrary_local_inference,
    aMLLibrary_cloud_inference
  };

  /// Collection of parameters for a device
  struct Device
  {
    // Device id
    std::size_t id;
    // Device name
    std::string name;
    // Maximum memory capacity (in bytes)
    memory_type maximum_memory;
    // The .csv file of weights
    std::string weights_path;
  };

  struct network_domain
  {
    std::string name;

    std::size_t bandwidth;
    double      access_delay;

    std::size_t depth;
  };

  struct device
  {
    std::string name;
    std::string domain_name;

    std::size_t ram;
    std::size_t vram;

    std::size_t id;
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
    std::string temporary_directory = "tmp";

    /// The (absolute) path to the onnx_tool package (if not default)
    std::string package_onnx_tool_location;

    std::string package_aMLLibrary_location;

    /// The number of paths to return
    std::size_t K;

    /// The KSP method
    KSP_Method method;

    /// Starting device
    std::size_t starting_device_id;

    /// End device
    std::size_t ending_device_id;

    /// Are backward collection allowed? (i.e. can device i send data to device j with i>j?)
    bool backward_connections_allowed;

    /// Mode for weight import
    Weight_Import_Mode weight_import_mode;

    /// The features of the .csv file to feed to aMLLibrary
    std::vector<std::string> weight_csv_features;

    /// Do we have to check for memory constraints?
    bool memory_constraint;

    /// The type of memory constraint
    Memory_Constraint_Type memory_constraint_type;

    /// The collection of devices
    std::vector<Device> devices;

    /// The bandwidth information between the different devices (Mbps - s)
    std::map<std::pair<std::size_t, std::size_t>, std::pair<bandwidth_type, bandwidth_type>> bandwidth;
  };
} // namespace network_butcher_parameters


#endif // NETWORK_BUTCHER_PARAMETERS_H
