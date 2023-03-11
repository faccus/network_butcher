#include "General_Manager.h"

/*
#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
#include <pybind11/iostream.h>
#include <pybind11/embed.h>
*/

/*
int
main(int argc, char **argv)
{
  network_butcher_io::General_Manager::read_command_line(argc, argv);

  return 0;
}*/
int main() {
  network_butcher_io::General_Manager::boot("test_data/configs/test5_parameters.conf", true);

  return 0;
}
/*
int main() {
  using namespace pybind11::literals;
  namespace py = pybind11;

  py::scoped_interpreter guard{};

  py::object path     = py::module_::import("sys");
  py::object inserter = path.attr("path").attr("append");
  inserter("/home/faccus/.local/lib/python3.10/site-packages");
  inserter("/home/faccus/CLionProjects/network_butcher");

  py::object test_1 = py::module_::import("onnx_tool");
  py::object test_2 = py::module_::import("aMLLibrary");

  return 0;
}
*/