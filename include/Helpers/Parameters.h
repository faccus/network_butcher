//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_PARAMETERS_H
#define NETWORK_BUTCHER_PARAMETERS_H

#include "Traits/Graph_traits.h"

/// Collection of parameters for a device
struct Device {
  // Device id
  std::size_t id;
  // Device name
  std::string name;
  // Maximum memory capacity (in bytes)
  memory_type maximum_memory;
  // The .csv file of weights
  std::string weights_path;
};

/// Enumerator for the different KSP methods
enum KSP_Method {
  Eppstein,
  Lazy_Eppstein
};

/// The memory constraints possibilities
enum Memory_Constraint_Type {
  None,
  Max,
  Preload_Parameters
};

/// The collection of parameters
struct Parameters {
  // The model name
  std::string model_name;

  // The model path
  std::string model_path;

  // The export directory
  std::string export_directory;

  // The number of paths to return
  std::size_t K;

  // The KSP method
  KSP_Method method;

  // Are backward collection allowed? (i.e. can device i send data to device j with i>j?)
  bool backward_connections_allowed;

  // Do we have to check for memory constraints?
  bool memory_constraint;

  // The type of memory constraint
  Memory_Constraint_Type memory_constraint_type;

  // The collection of devices
  std::vector<Device> devices;

  // The bandwidth information between the different devices (in Mbps)
  std::map<std::pair<std::size_t, std::size_t>, bandwidth_type> bandwidth;
};


#endif // NETWORK_BUTCHER_PARAMETERS_H
