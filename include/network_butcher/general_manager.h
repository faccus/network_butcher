#ifndef NETWORK_BUTCHER_MANAGER_H
#define NETWORK_BUTCHER_MANAGER_H

#include <network_butcher/APSC/chrono.h>
#include <network_butcher/io_manager.h>

namespace network_butcher::io
{
  /// \namespace General_Manager General_Manager is the 'main' namespace of the program. Indeed, here are stored the
  /// functions that execute the 'butchering' workflow as well as the function that can interact with the inputs of a
  /// program (if it is built). The boot method reads the parameter file and performs the butchering (import of the network,
  /// butchering and model reconstruction) while read_command_line, using GetPot, will read the arguments from the
  /// command line
  namespace General_Manager
  {
    namespace Helper_Functions
    {
      /// Based on the information related to the bandwidth between devices, it will produce the transmission function
      /// \param weights_params The collection of weight parameters
      /// \param graph The graph
      /// \return  The transmission function
      auto
      generate_bandwidth_transmission_function(const network_butcher::parameters::Parameters::Weights &weights_params,
                                               const Converted_Onnx_Graph_Type                        &graph)
        -> std::function<Time_Type(const Edge_Type &, size_t, size_t)>;


      /// It prints the help/usage message in the console
      void
      print_help();
    } // namespace Helper_Functions


    /// Boot! It will firstly import the specified network from the .onnx file. Then, the butchering process starts:
    /// based on the specified parameters, different partitions of the network will be produced and exported to the
    /// specified location.
    /// \param path The input configuration file
    /// \param performance Print some performance information (required time for each "phase")
    void
    boot(std::string const &path, bool performance = false);


    /// Boot! It will firstly import the specified network from the .onnx file. Then, the butchering process starts:
    /// based on the imported parameters, different partitions of the network will be produced and exported to the
    /// specified location.
    /// \param params The collection of parameters
    /// \param performance Print some performance information (required time for each "phase")
    void
    boot(network_butcher::parameters::Parameters const &params, bool performance = false);


    /// It reads the inputs given to the program, it will try import the parameters and call the boot method
    /// \param argc The number of arguments
    /// \param argv The arguments
    void
    read_command_line(int argc, char **argv);
  }; // namespace General_Manager
} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_MANAGER_H
