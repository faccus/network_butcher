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

struct device
{
  std::string name;
  std::string domain_name;

  std::size_t ram;
  std::size_t vram;

  std::size_t id;
};


int
main()
{
  std::map<std::string, network_domain> network_domains;
  std::map<std::string, std::string>    subdomain_to_domain;
  std::map<std::string, std::size_t>    domain_to_depth;

  std::map<std::string, device> devices_map;

  // Reads the candidate_resources file and tries to construct the network domain hierarchy. Moreover, it will produce
  // the list of avaible resources
  {
    auto       resources_file       = YAML::LoadFile("candidate_resources.yaml");
    YAML::Node network_domains_yaml = resources_file["System"]["NetworkDomains"];

    for (auto const &domain : network_domains_yaml)
      {
        auto const name = domain.first.as<std::string>();

        network_domains[name] = {domain.second["name"].as<std::string>(),
                                 domain.second["Bandwidth"].as<std::size_t>(),
                                 domain.second["AccessDelay"].as<double>()};

        if (domain_to_depth.find(name) == domain_to_depth.cend())
          domain_to_depth[name] = 0;

        if (domain.second["subNetworkDomains"].size() > 0)
          {
            for (auto const &subdomain : domain.second["subNetworkDomains"])
              {
                auto const subdomain_name = subdomain.as<std::string>();

                subdomain_to_domain[subdomain_name] = name;
                domain_to_depth[subdomain_name]     = domain_to_depth[name] + 1;
              }
          }

        if (domain.second["ComputationalLayers"])
          {
            YAML::Node layers = domain.second["ComputationalLayers"];
            for (YAML::const_iterator layer_it = layers.begin(); layer_it != layers.end(); ++layer_it)
              {
                auto const id = layer_it->second["number"].as<std::size_t>();

                if (layer_it->second["Resources"])
                  {
                    YAML::Node resources = layer_it->second["Resources"];
                    for (YAML::const_iterator resource_it = resources.begin(); resource_it != resources.end();
                         ++resource_it)
                      {
                        device dev;

                        dev.name = resource_it->second["name"].as<std::string>();
                        dev.id   = id;

                        dev.ram         = 0;
                        dev.vram        = 0;
                        dev.domain_name = name;

                        if (resource_it->second["memorySize"])
                          {
                            dev.ram = resource_it->second["memorySize"].as<std::size_t>();
                          }

                        if (resource_it->second["accelerators"])
                          {
                            YAML::Node accelerators = resource_it->second["accelerators"];
                            for (YAML::const_iterator it = accelerators.begin(); it != accelerators.end(); ++it)
                              {
                                dev.vram = std::max(dev.vram, it->second["memory"].as<std::size_t>());
                              }
                          }

                        devices_map.insert({resource_it->second["name"].as<std::string>(), dev});
                      }
                  }
              }
          }
      }
  }

  YAML::Node annotations = YAML::LoadFile("annotations.yaml"); // candidate_deployments candidate_resources
  YAML::Node components  = YAML::LoadFile("candidate_deployments.yaml")["Components"];

  std::map<std::string, std::pair<std::size_t, std::size_t>> to_deploy;

  // It will list the different models that have to be partitioned
  for (YAML::const_iterator it = annotations.begin(); it != annotations.end(); ++it)
    {
      if (it->second["partitionable_model"])
        {
          std::cout << it->first << ": " << it->second["component_name"]["name"] << std::endl;

          std::size_t ram  = 0;
          std::size_t vram = 0;

          if (it->second["device_constraints"])
            {
              YAML::Node n = it->second["device_constraints"];

              if (n["ram"])
                ram = n["ram"].as<std::size_t>();
              if (n["vram"])
                vram = n["vram"].as<std::size_t>();
            }

          to_deploy.insert({it->second["component_name"]["name"].as<std::string>(), {ram, vram}});
        }
    }


  auto const find_bandwidth = [&network_domains, &domain_to_depth, &subdomain_to_domain](std::string first_domain,
                                                                                         std::string second_domain) {
    auto first_domain_depth  = domain_to_depth[first_domain];
    auto second_domain_depth = domain_to_depth[second_domain];

    if (second_domain_depth > first_domain_depth)
      {
        std::swap(first_domain_depth, second_domain_depth);
        std::swap(first_domain, second_domain);
      }

    while (first_domain_depth > second_domain_depth)
      {
        if (first_domain == second_domain)
          {
            auto const &dom = network_domains[first_domain];
            return std::pair{dom.bandwidth, dom.access_delay};
          }

        --first_domain_depth;
        first_domain = subdomain_to_domain[first_domain];
      }

    while (first_domain_depth > 0)
      {
        if (first_domain == second_domain)
          {
            auto const &dom = network_domains[first_domain];
            return std::pair{dom.bandwidth, dom.access_delay};
          }

        --first_domain_depth;
        first_domain  = subdomain_to_domain[first_domain];
        second_domain = subdomain_to_domain[second_domain];
      }

    if (first_domain == second_domain)
      {
        auto const &dom = network_domains[first_domain];
        return std::pair{dom.bandwidth, dom.access_delay};
      }
    else
      {
        std::size_t const n  = 0;
        double const      ty = -1.;

        return std::pair{n, ty};
      }
  };

  for (auto const &model : to_deploy)
    {
      auto const &[model_friendly_name, pair_ram_vram] = model;
      auto const &[model_ram, model_vram]              = pair_ram_vram;

      std::vector<std::map<std::string, std::size_t>> devices_ram;

      for (YAML::const_iterator it = components.begin(); it != components.end(); ++it)
        {
          if (it->second["name"] &&
              it->second["name"].as<std::string>().find(model_friendly_name) != std::string::npos &&
              it->second["name"].as<std::string>().find("partitionX") != std::string::npos)
            {
              devices_ram.emplace_back();

              // Containers that can be deployed on the different resources
              YAML::Node devices = it->second["Containers"];
              for (YAML::const_iterator device_it = devices.begin(); device_it != devices.end(); ++device_it)
                {
                  devices_ram.back()[device_it->second["candidateExecutionResources"][0].as<std::string>()] =
                    std::max(device_it->second["memorySize"].as<std::size_t>(), model_ram);
                }
            }
        }

      // Remove the devices with insufficient ram and vram
      for (auto it = devices_ram.begin(); it != devices_ram.end(); ++it)
        {
          std::set<std::string> to_remove;
          for (auto const &[name, ram] : *it)
            {
              auto const &dev = devices_map[name];
              if (dev.ram < ram || dev.vram < model_vram)
                to_remove.insert(name);
            }

          for (auto const &name : to_remove)
            {
              it->erase(it->find(name));
            }
        }

      // If there is more than one "feasible" device
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

          // Prepare the collection of parameters for the current set of devices
          for (auto const &devices : device_for_partitions)
            {
              Parameters params;
              params.model_name = model_friendly_name;
              params.model_path = "";

              params.starting_device_id = 0;
              params.ending_device_id   = 0;

              params.method                       = Lazy_Eppstein;
              params.K                            = 100;
              params.backward_connections_allowed = false;

              std::size_t k = 0;
              for (auto const &device : devices)
                {
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
                      auto const &first_domain  = devices_map[devices[i].first].domain_name;
                      auto const &second_domain = devices_map[devices[j].first].domain_name;

                      params.bandwidth[{i, j}] = find_bandwidth(first_domain, second_domain);
                    }
                }

              std::cout << std::endl;
            }


          std::cout << std::endl;
        }
    }


  return 0;
}