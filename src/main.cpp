#include "../include/APSC/GetPot"
#include "../include/General_Manager.h"


int
main(int argc, char **argv)
{
  network_butcher_io::General_Manager::read_command_line(argc, argv);

  return 0;
}

/*
using namespace pybind11::literals;

int main(int argc, char ** argv) {
  py::object exp_pi = pi.attr("exp")();
  py::print(py::str(exp_pi));
}*/