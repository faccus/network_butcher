//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_MANAGER_H
#define NETWORK_BUTCHER_MANAGER_H

#include "IO_Manager.h"
#include "chrono.h"

namespace network_butcher::io
{
  /// General_Manager is the main namespace of the program. In fact, here the starting methods of the program are
  /// contained. The boot method reads the parameter file and performs the butchering (import of the network,
  /// butchering and model reconstruction)
  namespace General_Manager
  {
    namespace Helper_Functions
    {

      /// Based on the information related to the bandwidth between devices, it will produce a "transmission"
      /// function for the given case
      /// \param params The collection of parameters
      /// \param graph The graph
      /// \return  The transmission function
      std::function<weight_type(const edge_type &, size_t, size_t)>
      generate_bandwidth_transmission_function(const network_butcher::parameters::Parameters::Weights &weights_params,
                                               const graph_type                                       &graph);


      /// It prints the help/usage message in the console
      void
      print_help();
    } // namespace Helper_Functions


    /// Boot! It will firstly import the specified network from the .onnx file. Then, the butchering process starts:
    /// based on the specific parameters, different partitions of the network will be produced and exported to the
    /// specified location.
    /// \param path The input configuration file
    /// \param performance Print some performance information (required time for each "phase")
    void
    boot(std::string const &path, bool performance = false);


    /// Boot! It will firstly import the specified network from the .onnx file. Then, the butchering process starts:
    /// based on the specific parameters, different partitions of the network will be produced and exported to the
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
