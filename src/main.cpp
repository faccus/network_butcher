#include "General_Manager.h"

/*
int
main(int argc, char **argv)
{
  network_butcher_io::General_Manager::read_command_line(argc, argv);

  return 0;
}*/

int main() {
  auto const param = network_butcher_io::IO_Manager::read_parameters("test_data/configs/test5_parameters.conf");

  auto graph = std::get<0>(
    network_butcher_io::IO_Manager::import_from_onnx(param.model_path, false, true, 3, true));

  network_butcher_io::IO_Manager::utilities::import_weights_aMLLibrary_local(graph, param);

  return 0;
}