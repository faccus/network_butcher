
#include "../include/APSC/GetPot"
#include "../include/General_Manager.h"

int
main(int argc, char** argv)
{
  GetPot command_line(argc, argv);
  std::string const config_path = command_line("config_file", "config.conf");

  General_Manager::boot(config_path, true);

  return 0;
}
/*

#include "yaml-cpp/yaml.h"
#include <iostream>

int main()
{
  YAML::Node config = YAML::LoadFile("CandidateResources.yaml");


  std::cout << "Test" << std::endl;

  return 0;
}*/