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

int
main()
{
  std::string const annotations_path           = "annotations.yaml";
  std::string const candidate_deployments_path = "candidate_deployments.yaml";
  std::string const candidate_resources_path   = "candidate_resources.yaml";

  auto const params = network_butcher_io::IO_Manager::read_parameters_yaml(candidate_resources_path,
                                                                           candidate_deployments_path,
                                                                           annotations_path);


  return 0;
}