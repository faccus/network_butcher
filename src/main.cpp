#include "../include/Helpers/APSC/GetPot"
#include "../include/Helpers/General_Manager.h"

int
main(int argc, char** argv)
{
  GetPot command_line(argc, argv);
  std::string const config_path = command_line("config_file", "config.conf");

  General_Manager::boot(config_path);

  return 0;
}