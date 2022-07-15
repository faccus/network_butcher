//
// Created by faccus on 7/12/22.
//

#ifndef NETWORK_BUTCHER_MANAGER_H
#define NETWORK_BUTCHER_MANAGER_H

#include "IO_Manager.h"

class General_Manager {
private:

  static std::function<weight_type(const node_id_type &, size_t, size_t)>
  generate_bandwidth_transmission_function(const Parameters          &params,
                                           const Butcher<graph_type> &butcher);

  static void import_weights(graph_type &graph, Parameters const &params);

public:
  static void boot(std::string const &path);
};

#endif // NETWORK_BUTCHER_MANAGER_H
