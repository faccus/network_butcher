#ifndef NETWORK_BUTCHER_PARAMETERS_H
#define NETWORK_BUTCHER_PARAMETERS_H

#include <network_butcher/Network/graph_traits.h>
#include <network_butcher/K-shortest_path/ksp_method.h>

namespace network_butcher::parameters
{
  /// Enumerator for the weight import modes
  enum struct Weight_Import_Mode
  {
    /// Single file, simple graph
    single_direct_read,

    /// Multiple files, simple graph
    multiple_direct_read,

    /// aMLLibrary generates the weights for the block graph
    aMLLibrary_block,

    /// Single file, block graph
    block_single_direct_read,

    /// Multiple files, block graph
    block_multiple_direct_read
  };

  /// Enumerator for the block graph generation modes
  enum struct Block_Graph_Generation_Mode
  {
    /// Bottleneck input and output are not in the block node
    classic,

    /// Bottleneck output is in the block node, bottleneck input is not
    output,

    /// Bottleneck input is in the block node, bottleneck output is not
    input
  };

  /// Collection of parameters for a device
  struct Device
  {
    /// Device id
    std::size_t id;

    /// Device name
    std::string name;

    /// Maximum memory capacity (in bytes)
    Memory_Type maximum_memory;

    /// The .csv file of weights
    std::string weights_path;

    /// The column of the .csv file containing the weights
    std::string relevant_entry;
  };

  /// The collection of parameters
  struct Parameters
  {
    using Devices = std::vector<Device>;

    /// Structure used to contain all the aMLLibrary related parameters
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

    /// Structure used to contain all the weights related parameters
    struct Weights
    {
      using connection_information_type = std::pair<Bandwidth_Value_Type, Access_Delay_Value_Type>;
      using connection_type             = std::unique_ptr<
        network_butcher::types::WGraph<false, network_butcher::types::Node, connection_information_type>>;
      using io_connection_type = std::map<Edge_Type, connection_information_type>;

      /// Mode for weight import
      Weight_Import_Mode weight_import_mode;

      /// The path of the .csv file that stores all the weights
      std::string single_weight_import_path;

      /// The columns of the .csv file that store the weights
      std::vector<std::string> single_csv_columns_weights;

      /// The separator
      char separator;

      /// The bandwidth information between the different devices (Mbps , s).
      connection_type    bandwidth;

      /// The bandwidth information between the devices for the first padding node and its neighbours (Mbps , s).
      io_connection_type in_bandwidth;

      /// The bandwidth information between the devices for the last padding node and its neighbours (Mbps , s).
      io_connection_type out_bandwidth;
    };

    /// Structure used to contain all the model related parameters
    struct Model
    {
      /// The model name
      std::string model_name;

      /// The model path
      std::string model_path;

      /// The export directory
      std::string export_directory;

      /// The config path
      std::string config_path;
    };

    /// Structure used to contain all the parameters related to the K shortest path method
    struct KSP
    {
      /// The number of paths to return
      std::size_t K;

      /// The KSP method
      KSP_Method method;
    };

    /// Structure used to contain all the parameters related to the Block Graph Generation
    struct Block_Graph_Generation
    {
      /// Starting device
      std::size_t starting_device_id;

      /// End device
      std::size_t ending_device_id;

      /// Block Graph Generation mode for Butcher.
      Block_Graph_Generation_Mode block_graph_mode;

      /// Do we have to use the bandwidth to manage connections, i.e. determine if two devices can communicate?
      bool use_bandwidth_to_manage_connections;

      /// Do we have to check for memory constraints? For more details on its usage, refer to the Memory_Constraint class
      bool memory_constraint;
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
