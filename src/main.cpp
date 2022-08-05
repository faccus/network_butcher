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
      std::map<std::string, std::size_t> devices_ram;
      std::set<std::size_t>              layers;

      for (YAML::const_iterator it = components.begin(); it != components.end(); ++it)
        {
          if (it->second["name"] &&
              it->second["name"].as<std::string>().find(model_friendly_name) != std::string::npos &&
              it->second["name"].as<std::string>().find("partitionX") != std::string::npos)
            {
              YAML::Node execution_layers = it->second["candidateExecutionLayers"];


              for (auto const &idw : execution_layers)
                {
                  layers.emplace(idw.as<std::size_t>());
                }

              YAML::Node devices = it->second["Containers"];

              for (YAML::const_iterator device_it = devices.begin(); device_it != devices.end(); ++device_it)
                {
                  devices_ram[device_it->second["candidateExecutionResources"][0].as<std::string>()] =
                    device_it->second["memorySize"].as<std::size_t>();
                }
            }
        }

      Parameters params;
      params.devices = std::vector<Device>(devices_ram.size());

      {
        std::size_t index = 0;
        for (auto const &device : devices_ram)
          {
            auto &ref          = params.devices[index];
            ref.maximum_memory = device.second;
            ref.id             = index++;
          }
      }
    }


  return 0;
}