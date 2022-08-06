/*
#include "../include/APSC/GetPot"
#include "../include/General_Manager.h"

int
main(int argc, char **argv)
{
  GetPot            command_line(argc, argv);
  std::string const config_path = command_line("config_file", "config.conf");

  network_butcher_io::General_Manager::boot(config_path, true);

  return 0;
}
*/

#include "../include/IO_Manager.h"

#include <iostream>
#include <yaml-cpp/yaml.h>

struct network_domain
{
  std::string name;

  std::size_t bandwidth;
  double      access_delay;
};


int
main()
{
  std::map<std::string, network_domain>        network_domains;
  std::map<std::string, std::set<std::string>> subdomains;

  std::map<std::size_t, std::string> layer_to_domain;
  std::map<std::string, std::size_t> name_to_layer;

  {
    YAML::Node network_domains_yaml = YAML::LoadFile("candidate_resources.yaml")["System"]["NetworkDomains"];
    for (auto const &domain : network_domains_yaml)
      {
        auto const name = domain.first.as<std::string>();

        network_domains[name] = {domain.second["name"].as<std::string>(),
                                 domain.second["Bandwidth"].as<std::size_t>(),
                                 domain.second["AccessDelay"].as<double>()};

        if (domain.second["subNetworkDomains"].size() > 0)
          {
            for (auto const &subdomain : domain.second["subNetworkDomains"])
              subdomains[name].insert(subdomain.as<std::string>());
          }

        if (domain.second["ComputationalLayers"])
          {
            YAML::Node layers = domain.second["ComputationalLayers"];
            for (YAML::const_iterator layer_it = layers.begin(); layer_it != layers.end(); ++layer_it)
              {
                auto const id       = layer_it->second["number"].as<std::size_t>();
                layer_to_domain[id] = name;

                if (layer_it->second["Resources"])
                  {
                    YAML::Node resources = layer_it->second["Resources"];
                    for (YAML::const_iterator resource_it = resources.begin(); resource_it != resources.end();
                         ++resource_it)
                      {
                        name_to_layer[resource_it->second["name"].as<std::string>()] = id;
                      }
                  }
              }
          }
      }
  }

  YAML::Node annotations = YAML::LoadFile("annotations.yaml"); // candidate_deployments candidate_resources
  YAML::Node components  = YAML::LoadFile("candidate_deployments.yaml")["Components"];

  std::set<std::string> to_deploy;

  for (YAML::const_iterator it = annotations.begin(); it != annotations.end(); ++it)
    if (it->second["partitionable_model"])
      {
        std::cout << it->first << ": " << it->second["component_name"]["name"] << std::endl;
        to_deploy.emplace(it->second["component_name"]["name"].as<std::string>());
      }


  for (auto const &model_friendly_name : to_deploy)
    {
      std::vector<std::map<std::string, std::size_t>> devices_ram;
      std::vector<std::set<std::size_t>>              layers;
      std::size_t                                     exe_layer = 0;

      for (YAML::const_iterator it = components.begin(); it != components.end(); ++it)
        {
          if (it->second["name"] &&
              it->second["name"].as<std::string>().find(model_friendly_name) != std::string::npos &&
              it->second["name"].as<std::string>().find("partitionX") != std::string::npos)
            {
              devices_ram.emplace_back();
              layers.emplace_back();

              YAML::Node execution_layers = it->second["candidateExecutionLayers"];


              for (auto const &idw : execution_layers)
                {
                  layers[exe_layer].emplace(idw.as<std::size_t>());
                }

              YAML::Node devices = it->second["Containers"];

              for (YAML::const_iterator device_it = devices.begin(); device_it != devices.end(); ++device_it)
                {
                  devices_ram.back()[device_it->second["candidateExecutionResources"][0].as<std::string>()] =
                    device_it->second["memorySize"].as<std::size_t>();
                }

              ++exe_layer;
            }
        }

      if (devices_ram.size() > 1)
        {
          std::vector<std::vector<std::pair<std::string, std::size_t>>> device_for_partitions;
          device_for_partitions.emplace_back();

          for (std::size_t i = 0; i < devices_ram.size(); ++i)
            {
              auto const size = device_for_partitions.size();
              for (std::size_t k = 0; k < size; ++k)
                {
                  auto &partition = device_for_partitions[k];
                  if (!devices_ram[i].empty())
                    {
                      auto const copy = partition;
                      auto       it   = devices_ram[i].begin();
                      partition.push_back({it->first, it->second});
                      ++it;
                      for (; it != devices_ram[i].end(); ++it)
                        {
                          device_for_partitions.push_back(copy);
                          device_for_partitions.back().push_back({it->first, it->second});
                        }
                    }
                }
            }

          for (auto const devices : device_for_partitions)
            {
              Parameters params;
              params.model_name = model_friendly_name;
              params.model_path = "";

              params.starting_device_id = 0;
              params.ending_device_id   = 0;

              params.method                       = Lazy_Eppstein;
              params.K                            = 100;
              params.backward_connections_allowed = false;


              std::vector<std::string> domains;
              domains.reserve(devices.size());

              std::size_t k = 0;
              for (auto const &device : devices)
                {
                  domains.push_back(layer_to_domain[name_to_layer[device.first]]);
                  params.devices.emplace_back();

                  auto &dev          = params.devices.back();
                  dev.id             = k++;
                  dev.maximum_memory = device.second;
                  dev.name           = device.first;
                  dev.weights_path   = "";
                }

              for (std::size_t i = 0; i < params.devices.size(); ++i)
                {
                  for (std::size_t j = i + 1; j < params.devices.size(); ++j)
                    {
                      if (domains[i] == domains[j])
                        {
                          auto const &dom          = network_domains[domains[i]];
                          params.bandwidth[{i, j}] = {dom.bandwidth, dom.access_delay};
                        }
                      else
                        {
                          // Add research of parent ND...
                        }
                    }
                }

              std::cout << std::endl;
            }


          std::cout << std::endl;
        }
    }


  return 0;
}