
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

/*
#include "yaml-cpp/yaml.h"

#include <iostream>

int
main()
{
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
      for (YAML::const_iterator it = components.begin(); it != components.end(); ++it)
        {
          if (it->second["name"] &&
              it->second["name"].as<std::string>().find(model_friendly_name) != std::string::npos &&
              it->second["name"].as<std::string>().find("partitionX") != std::string::npos)
            {}
        }
    }
  for (YAML::const_iterator it = components.begin(); it != components.end(); ++it)
    {
      if (it->second["name"] && to_deploy.contains(it->second["name"].as<std::string>()))
        {
          YAML::Node execution_layers = it->second["candidateExecutionLayers"];

          std::set<std::size_t> layers;
          for (YAML::const_iterator it2 = execution_layers.begin(); it2 != execution_layers.end(); ++it2)
            layers.emplace(it2->second.as<std::size_t>());

          YAML::Node devices = it->second["Containers"];
          for (YAML::const_iterator device_it = devices.begin(); device_it != devices.end(); ++device_it)
            {}
        }
    }


  return 0;
}*/