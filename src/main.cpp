
#include "../include/APSC/GetPot"
#include "../include/General_Manager.h"

int
main(int argc, char **argv)
{
  GetPot command_line(argc, argv);

  std::cout << "Network Butcher v1.0" << std::endl;

  if (command_line.size() == 2)
    {
      if (command_line.argument_contains(0, "help"))
        {
          std::cout << std::endl << "Command usage: " << std::endl;
          std::cout << "#1: ./network_butcher config_file=config.conf" << std::endl;
          std::cout << "#2: ./network_butcher annotations=annotations.yaml "
                       "candidate_deployments=candidate_deployments.yaml candidate_resources=candidate_resources.yaml"
                    << std::endl;
        }
      else
        {
          std::string const config_path = command_line("config_file", "config.conf");

          network_butcher_io::General_Manager::boot(config_path, true);
        }
    }
#if YAML_CPP_ACTIVE
  else if (command_line.size() == 4)
    {
      std::string const annotations_path = command_line("annotations", "annotations.yaml");
      std::string const candidate_deployments_path =
        command_line("candidate_deployments", "candidate_deployments.yaml");
      std::string const candidate_resources_path = command_line("candidate_resources", "candidate_resources.yaml");

      auto params = network_butcher_io::IO_Manager::read_parameters_yaml(candidate_resources_path,
                                                                         candidate_deployments_path,
                                                                         annotations_path);

      for (std::size_t i = 0; i < params.size(); ++i)
        {
          auto &param = params[i];

          param.export_directory = std::to_string(i);
          network_butcher_io::General_Manager::boot(param, true);
        }
    }
#endif

  return 0;
}