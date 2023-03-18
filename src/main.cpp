#include "General_Manager.h"
/*
int
main(int argc, char **argv)
{
  network_butcher_io::General_Manager::read_command_line(argc, argv);

  return 0;
}
*/

int
main()
{
  network_butcher_io::General_Manager::boot("test_data/configs/test5_parameters.conf", true);

  return 0;
}