#include "../include/Helpers/APSC/GetPot"
#include "../include/Helpers/IO_Manager.h"

#include <iostream>

int
main(int argc, char** argv)
{
  GetPot command_line(argc, argv);
  std::string const path = command_line("generate_regression", "");
  std::string const out_path =
    command_line("output_file", "butcher_predict.csv");
  std::string const file_path = command_line("input_file", "");

  if(!path.empty() && !file_path.empty())
    {
      auto pair = IO_Manager::import_from_onnx(file_path);
      IO_Manager::export_network_infos_to_csv(std::get<0>(pair),
                                              std::get<1>(pair),
                                              out_path);
    }

  return 0;
}