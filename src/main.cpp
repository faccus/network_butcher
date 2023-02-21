#include "General_Manager.h"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

/*
int
main(int argc, char **argv)
{
  network_butcher_io::General_Manager::read_command_line(argc, argv);

  return 0;
}
*/

using namespace pybind11::literals;
namespace py = pybind11;

int
main(int argc, char **argv)
{
  py::scoped_interpreter guard{};

  std::string const packages = "/home/faccus/.local/lib/python3.10/site-packages/";
  std::string const model_path = "test_data/models/resnet18-v2-7-inferred.onnx";

  py::object sys_path = py::module_::import("sys").attr("path");
  py::object insert_path = sys_path.attr("insert");
  insert_path(0, packages);

  // start the interpreter and keep it alive
  py::object onnx_tool = py::module_::import("onnx_tool");

  py::object model_profile = onnx_tool.attr("model_profile");
  model_profile(model_path, "savenode"_a="test_data/network_stats.csv");

  return 0;
}