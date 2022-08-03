//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_MANAGER_H
#define NETWORK_BUTCHER_MANAGER_H

#include "IO_Manager.h"
#include "APSC/chrono.h"

class General_Manager {
private:

  /// Based on the information related to the bandwidth between devices, it will
  /// produce a "transmission" function for the given case
  /// \param params The collection of parameters
  /// \param graph The graph
  /// \return The transmission function
  static std::function<weight_type(const node_id_type &, size_t, size_t)>
  generate_bandwidth_transmission_function(const Parameters          &params,
                                           const graph_type &graph);

  /// It will import into the graph the different collection of weights
  /// \param graph The graph
  /// \param params The collection of parameters
  static void import_weights(graph_type &graph, Parameters const &params);

public:
  /// Boot! It will read the parameters from the given file and it will import
  /// the specified network from the .onnx file. Then, the butchering process
  /// starts: based on the specific parameters, different partitions of the
  /// network will be produced and exported to the specified location.
  /// \param path The input configuration file
  static void
  boot(std::string const &path, bool performance = false);


  /// Boot! It will firstly import the specified network from the .onnx file.
  /// Then, the butchering process starts: based on the specific parameters,
  /// different partitions of the network will be produced and exported to the
  /// specified location.
  /// \param path The input configuration file
  static void
  boot(Parameters const &params, bool performance = false);
};

#endif // NETWORK_BUTCHER_MANAGER_H
