//
// Created by faccus on 9/8/22.
//

#ifndef NETWORK_BUTCHER_YAML_IMPORTER_HELPERS_H
#define NETWORK_BUTCHER_YAML_IMPORTER_HELPERS_H

#include "../Traits/Graph_traits.h"
#include "../Types/Parameters.h"

#include <tuple>
#include <yaml-cpp/yaml.h>


namespace network_butcher_io
{
  class Yaml_importer_helpers
  {
  public:
    static std::
      tuple<std::map<std::string, network_domain>, std::map<std::string, std::string>, std::map<std::string, device>>
      read_candidate_resources(const std::string &candidate_resources_path);

    static std::pair<bandwidth_type, bandwidth_type>
    find_bandwidth(std::map<std::string, network_domain> const &network_domains,
                   std::map<std::string, std::string> const    &subdomain_to_domain,
                   std::string                                  first_domain,
                   std::string                                  second_domain);

    static std::map<std::string, std::pair<std::size_t, std::size_t>>
    read_annotations(const std::string &annotations_path);


    static std::vector<std::map<std::string, std::size_t>>
    read_candidate_deployments(YAML::Node const                    &components,
                               std::string const                   &model_name,
                               std::size_t                          model_ram,
                               std::size_t                          model_vram,
                               std::map<std::string, device> const &devices_map);

    static std::vector<std::vector<std::pair<std::string, std::size_t>>>
    get_devices_for_partitions(const std::vector<std::map<std::string, std::size_t>> &devices_ram);
  };
} // namespace network_butcher_io

#endif // NETWORK_BUTCHER_YAML_IMPORTER_HELPERS_H
