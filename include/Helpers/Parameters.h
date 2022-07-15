//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_PARAMETERS_H
#define NETWORK_BUTCHER_PARAMETERS_H

#include "Traits/Graph_traits.h"

struct Device {

  std::size_t id;
  std::string name;
  memory_type maximum_memory; // MB
  std::string weights_path;
};

enum KSP_Method {
  Eppstein,
  Lazy_Eppstein
};

enum Memory_Constraint_Type {
  Sum,
  Max
};

struct Parameters {
  std::string model_name;
  std::string model_path;
  std::string export_directory;

  std::size_t K;
  KSP_Method method;
  bool backward_connections_allowed;

  bool memory_constraint;
  Memory_Constraint_Type memory_constraint_type;


  std::vector<Device> devices;
  std::map<std::pair<std::size_t, std::size_t>, bandwidth_type> bandwidth; // Mbps
};


#endif // NETWORK_BUTCHER_PARAMETERS_H
