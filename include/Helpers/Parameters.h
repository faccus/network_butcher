//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_PARAMETERS_H
#define NETWORK_BUTCHER_PARAMETERS_H

#include "Traits/Graph_traits.h"

struct Device {

  std::size_t id;
  memory_type maximum_memory; // MB
  std::string weights_path;
};

enum KSP_Method {
  Eppstein,
  Lazy_Eppstein
};

struct Parameters {
  std::string model_name;
  std::string model_path;
  std::size_t K;
  bool backward_connections_allowed;
  std::string export_directory;
  KSP_Method method;

  std::vector<Device> devices;
  std::map<std::pair<std::size_t, std::size_t>, bandwidth_type> bandwidth; // Mbps
};


#endif // NETWORK_BUTCHER_PARAMETERS_H
