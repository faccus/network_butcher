//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_PARAMETERS_H
#define NETWORK_BUTCHER_PARAMETERS_H

#include "Graph_traits.h"

namespace network_butcher_parameters
{
  /// @brief Enumerator for the weight import modes
  enum Weight_Import_Mode
  {
    single_direct_read,
    multiple_direct_read,
    aMLLibrary_inference_original,
    aMLLibrary_inference_block
  };

  /// @brief Collection of parameters for a device
  struct Device
  {
    /// @brief Device id
    std::size_t id;

    /// @brief Device name
    std::string name;

    /// @brief Maximum memory capacity (in bytes)
    memory_type maximum_memory;

    /// @brief The .csv file of weights
    std::string weights_path;

    /// @brief The column of the .csv file containing the weights
    std::string relevant_entry;
  };

  /// @brief Enumerator for the different KSP methods
  enum KSP_Method
  {
    Eppstein,
    Lazy_Eppstein
  };

  /// @brief The memory constraints possibilities
  enum Memory_Constraint_Type
  {
    None,
    Max,
    Preload_Parameters
  };

  /// @brief The collection of parameters
  struct Parameters
  {
    /// @brief The model name
    std::string model_name;

    /// @brief The model path
    std::string model_path;

    /// @brief The export directory
    std::string export_directory;

    /// @brief The temporary directory
    std::string temporary_directory;

    /// @brief The (absolute) path to the onnx_tool package (if not default)
    std::vector<std::string> extra_packages_location;

    /// @brief The number of paths to return
    std::size_t K;

    /// @brief The KSP method
    KSP_Method method;

    /// @brief Starting device
    std::size_t starting_device_id;

    /// @brief End device
    std::size_t ending_device_id;

    /// @brief Are backward collection allowed? (i.e. can device j send data to device k with j>k?)
    bool backward_connections_allowed;

    /// @brief Mode for weight import
    Weight_Import_Mode weight_import_mode;

    /// @brief The variables of the .csv file that aMLLibrary should
    std::vector<std::string> aMLLibrary_inference_variables;

    /// @brief The features of the .csv file to feed to aMLLibrary
    std::vector<std::string> aMLLibrary_csv_features;

    /// @brief The path of the .csv file that stores all the weights
    std::string single_weight_import_path;

    /// @brief The columns of the .csv file that store the weights
    std::vector<std::string> single_csv_columns_weights;

    /// @brief The separator
    char separator;

    /// @brief Do we have to check for memory constraints?
    bool memory_constraint;

    /// @brief The type of memory constraint
    Memory_Constraint_Type memory_constraint_type;

    /// @brief The collection of devices
    std::vector<Device> devices;

    /// @brief The bandwidth information between the different devices (Mbps - s)
    std::map<std::pair<std::size_t, std::size_t>, std::pair<bandwidth_type, bandwidth_type>> bandwidth;
  };
} // namespace network_butcher_parameters


#endif // NETWORK_BUTCHER_PARAMETERS_H
