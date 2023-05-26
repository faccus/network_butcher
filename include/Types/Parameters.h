//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_PARAMETERS_H
#define NETWORK_BUTCHER_PARAMETERS_H

#include "Graph_traits.h"
#include "KSP_Method.h"

namespace network_butcher::parameters
{
  /// Enumerator for the weight import modes
  enum struct Weight_Import_Mode
  {
    single_direct_read,
    multiple_direct_read,
    aMLLibrary_original,

    aMLLibrary_block,
    block_single_direct_read,
    block_multiple_direct_read
  };

  enum struct Block_Graph_Generation_Mode
  {
    classic,
    output,
    input
  };

  /// The memory constraints possibilities
  enum Memory_Constraint_Type
  {
    None,
    Max,
    Preload_Parameters
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

  /// The collection of parameters
  struct Parameters
  {
    using Devices = std::vector<Device>;

    struct aMLLibrary
    {
      /// The temporary directory
      std::string temporary_directory;

      /// The (absolute) path to the onnx_tool package (if not default)
      std::vector<std::string> extra_packages_location;

      /// The variables of the .csv file that aMLLibrary should
      std::vector<std::string> aMLLibrary_inference_variables;

      /// The features of the .csv file to feed to aMLLibrary
      std::vector<std::string> aMLLibrary_csv_features;
    };

    struct Weights
    {
      using connection_information_type = std::pair<bandwidth_type, access_delay_type>;
      using connection_type             = std::unique_ptr<
        network_butcher::types::MWGraph<false, network_butcher::types::Node, connection_information_type>>;

      /// Mode for weight import
      Weight_Import_Mode weight_import_mode;

      /// The path of the .csv file that stores all the weights
      std::string single_weight_import_path;

      /// The columns of the .csv file that store the weights
      std::vector<std::string> single_csv_columns_weights;

      /// The separator
      char separator;

      /// The bandwidth information between the different devices (Mbps - s). The first map will be associated
      /// to standard connections between layers, the second one to the input padding node while the third one
      /// to the output padding node.
      connection_type bandwidth;
    };

    struct Model
    {
      /// The model name
      std::string model_name;

      /// The model path
      std::string model_path;

      /// The export directory
      std::string export_directory;
    };

    struct KSP
    {
      /// The number of paths to return
      std::size_t K;

      /// The KSP method
      KSP_Method method;
    };

    struct Block_Graph_Generation
    {
      /// Starting device
      std::size_t starting_device_id;

      /// End device
      std::size_t ending_device_id;

      /// Block Graph Generation mode for Butcher
      Block_Graph_Generation_Mode block_graph_mode;

      /// Do we have to check for memory constraints?
      bool memory_constraint;

      /// The type of memory constraint
      Memory_Constraint_Type memory_constraint_type;
    };

    /// aMMLibrary parameters
    aMLLibrary aMLLibrary_params;

    /// Weights parameters
    Weights weights_params;

    /// Model parameters
    Model model_params;

    /// KSP parameters
    KSP ksp_params;

    /// Block Graph Generation parameters
    Block_Graph_Generation block_graph_generation_params;

    /// The collection of devices
    Devices devices;
  };
} // namespace network_butcher::parameters


#endif // NETWORK_BUTCHER_PARAMETERS_H
