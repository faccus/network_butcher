#include "../include/APSC/GetPot"
#include "../include/General_Manager.h"

/*
#include <cpr/cpr.h>

int main(int argc, char** argv) {
  cpr::Response r = cpr::Post(cpr::Url{"localhost:8081"},
                              cpr::Payload{{"key", "value"}});
  std::cout << r.text << std::endl;          // JSON text string
}

*/

int
main(int argc, char **argv)
{
  GetPot command_line(argc, argv);

  std::cout << "Network Butcher v1.0" << std::endl;

  if (command_line.search(2, "--help", "-h"))
    {
      std::cout << std::endl << "Command usage: " << std::endl;
      std::cout << "#1: ./network_butcher config_file=config.conf" << std::endl;
      std::cout << "#2: ./network_butcher annotations=annotations.yaml "
                   "candidate_deployments=candidate_deployments.yaml candidate_resources=candidate_resources.yaml"
                << std::endl;
    }
  else
    {
      if (command_line.vector_variable_size("config_file"))
        {
          std::string const config_path = command_line("config_file", "config.conf");

          network_butcher_io::General_Manager::boot(config_path, true);
        }
      else if (command_line.vector_variable_size("annotations") ||
               command_line.vector_variable_size("candidate_deployments") ||
               command_line.vector_variable_size("candidate_resources"))
        {
#if YAML_CPP_ACTIVE
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

              param.export_directory = "ksp_result_yaml_" + std::to_string(i);
              network_butcher_io::General_Manager::boot(param, true);
            }
#else
          std::cout << "The library Yaml-Cpp is required to read .yaml files. Please, check the CMakeList.txt "
                       "configuration file."
                    << std::endl;
#endif
        }
    }

  return 0;
}
