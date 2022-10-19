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
    /// Reads the candidate deployments file and returns {network_domains, subdomain_to_domain, devices_map}
    /// \param candidate_resources_path The candidate resources file path
    /// \return Returns {network_domains, subdomain_to_domain, devices_map}
    static std::
      tuple<std::map<std::string, network_domain>, std::map<std::string, std::string>, std::map<std::string, device>>
      read_candidate_resources(const std::string &candidate_resources_path);

    /// Search for the bandwidth and network access time from the first domain to the second one
    /// \param network_domains The collection of network_domains
    /// \param subdomain_to_domain Map from the subdomains to the associated domains
    /// \param first_domain The first domain
    /// \param second_domain The second domain
    /// \return Bandwidth and network access time
    static std::pair<bandwidth_type, bandwidth_type>
    find_bandwidth(std::map<std::string, network_domain> const &network_domains,
                   std::map<std::string, std::string> const    &subdomain_to_domain,
                   std::string                                  first_domain,
                   std::string                                  second_domain);


    /// Reads the annotation file and extract the general information of the model to be partitioned
    /// \param annotations_path The annotation path
    /// \return Map that associates to the model name the avaible ram and vram
    static std::map<std::string, std::pair<std::size_t, std::size_t>>
    read_annotations(const std::string &annotations_path);


    /// Reads the candidate deployments file and returns the list of containers that can be deployed for every partition
    /// of every model
    /// \param candidate_deployments_path The candidate deployments file path
    /// \param models The collection of models (with ram and vram)
    /// \param devices_map The collection of devices
    /// \return A map that associates to every model the collection of devices for each possible partition
    static std::map<std::string, std::vector<std::map<std::string, std::size_t>>>
    read_candidate_deployments(std::string const                   &candidate_deployments_path,
                               std::map<std::string, std::pair<std::size_t, std::size_t>> const &models,
                               std::map<std::string, device> const &devices_map);


    static std::vector<std::vector<std::pair<std::string, std::size_t>>>
    get_devices_for_partitions(const std::vector<std::map<std::string, std::size_t>> &devices_ram);
  };
} // namespace network_butcher_io

#endif // NETWORK_BUTCHER_YAML_IMPORTER_HELPERS_H
