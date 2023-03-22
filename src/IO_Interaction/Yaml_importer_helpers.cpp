//
// Created by faccus on 9/8/22.
//
#include "Yaml_importer_helpers.h"


namespace network_butcher::io::Yaml_importer_helpers
{
  std::vector<std::vector<std::pair<std::string, std::size_t>>>
  get_devices_for_partitions(const std::vector<std::map<std::string, std::size_t>> &devices_ram)
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
                auto       it   = devices_ram[i].cbegin();
                partition.push_back({it->first, it->second});
                ++it;
                for (; it != devices_ram[i].cend(); ++it)
                  {
                    device_for_partitions.push_back(copy);
                    device_for_partitions.back().push_back({it->first, it->second});
                  }
              }
          }
      }

    return device_for_partitions;
  }


  std::map<std::string, std::vector<std::map<std::string, std::size_t>>>
  read_candidate_deployments(std::string const &candidate_deployments_path,
                             std::map<std::string, std::pair<std::size_t, std::size_t>> const &models,
                             std::map<std::string, network_device> const                      &devices_map)
  {
    YAML::Node components = YAML::LoadFile(candidate_deployments_path)["Components"];
    std::map<std::string, std::vector<std::map<std::string, std::size_t>>> final_res;

    for (auto const &[model_name, pair_ram_vram] : models)
      {
        auto const &[model_ram, model_vram] = pair_ram_vram;

        std::list<std::map<std::string, std::size_t>>   devices_ram;
        std::vector<std::map<std::string, std::size_t>> res;

        for (YAML::const_iterator it = components.begin(); it != components.end(); ++it)
          {
            if (it->second["name"] && it->second["name"].as<std::string>().find(model_name) != std::string::npos &&
                it->second["name"].as<std::string>().find("partitionX") != std::string::npos)
              {
                devices_ram.emplace_back();

                // Containers that can be deployed on the different resources
                YAML::Node devices = it->second["Containers"];
                for (YAML::const_iterator device_it = devices.begin(); device_it != devices.end(); ++device_it)
                  {
                    auto &candidate_execution_resources = device_it->second["candidateExecutionResources"];
                    for (YAML::const_iterator itt = candidate_execution_resources.begin();
                         itt != candidate_execution_resources.end();
                         ++itt)
                      {
                        devices_ram.back()[itt->as<std::string>()] =
                          std::max(device_it->second["memorySize"].as<std::size_t>(), model_ram);
                      }
                  }
              }
          }

        // Remove the devices with insufficient ram and vram
        for (auto it = devices_ram.begin(); it != devices_ram.end(); ++it)
          {
            std::set<std::string> to_remove;
            for (auto const &[name, ram] : *it)
              {
                auto const &dev = devices_map.find(name)->second;
                if (dev.ram < ram || dev.vram < model_vram)
                  to_remove.insert(name);
              }

            for (auto const &name : to_remove)
              {
                it->erase(it->find(name));
              }

            if (!it->empty())
              res.emplace_back(std::move(*it));
          }

        final_res.emplace(model_name, std::move(res));
      }

    return final_res;
  }


  std::map<std::string, std::pair<std::size_t, std::size_t>>
  read_annotations(const std::string &annotations_path)
  {
    std::map<std::string, std::pair<std::size_t, std::size_t>> to_deploy;

    YAML::Node annotations = YAML::LoadFile(annotations_path);

    // It will list the different models that have to be partitioned
    for (YAML::const_iterator it = annotations.begin(); it != annotations.end(); ++it)
      {
        if (it->second["partitionable_model"])
          {
            // std::cout << it->first << ": " << it->second["component_name"]["name"] << std::endl;

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

    return to_deploy;
  }


  std::pair<bandwidth_type, bandwidth_type>
  find_bandwidth(std::map<std::string, network_domain> const &network_domains,
                 std::map<std::string, std::string> const    &subdomain_to_domain,
                 std::string                                  first_domain,
                 std::string                                  second_domain)
  {
    auto first_domain_depth  = network_domains.find(first_domain)->second.depth;
    auto second_domain_depth = network_domains.find(second_domain)->second.depth;

    if (second_domain_depth > first_domain_depth)
      {
        std::swap(first_domain_depth, second_domain_depth);
        std::swap(first_domain, second_domain);
      }

    while (first_domain_depth > second_domain_depth)
      {
        if (first_domain == second_domain)
          {
            auto const &dom = network_domains.find(first_domain)->second;
            return std::pair{dom.bandwidth, dom.access_delay};
          }

        --first_domain_depth;
        first_domain = subdomain_to_domain.find(first_domain)->second;
      }

    while (first_domain_depth > 0)
      {
        if (first_domain == second_domain)
          {
            auto const &dom = network_domains.find(first_domain)->second;
            return std::pair{dom.bandwidth, dom.access_delay};
          }

        --first_domain_depth;
        first_domain  = subdomain_to_domain.find(first_domain)->second;
        second_domain = subdomain_to_domain.find(second_domain)->second;
      }

    if (first_domain == second_domain)
      {
        auto const &dom = network_domains.find(first_domain)->second;
        return std::pair{dom.bandwidth, dom.access_delay};
      }
    else
      return std::pair{0, -1.};
  }


  std::tuple<std::map<std::string, network_domain>,
             std::map<std::string, std::string>,
             std::map<std::string, network_device>>
  read_candidate_resources(const std::string &candidate_resources_path)
  {
    std::map<std::string, network_domain> network_domains;
    std::map<std::string, std::string>    subdomain_to_domain;
    std::map<std::string, std::size_t>    domain_to_depth;
    std::map<std::string, network_device> devices_map;

    auto       resources_file       = YAML::LoadFile(candidate_resources_path);
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
                        network_device dev;

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

    for (auto &domain : network_domains)
      {
        domain.second.depth = domain_to_depth[domain.first];
      }

    return {network_domains, subdomain_to_domain, devices_map};
  }
} // namespace network_butcher::io::Yaml_importer_helpers