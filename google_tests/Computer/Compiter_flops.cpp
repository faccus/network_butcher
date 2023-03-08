#include <gtest/gtest.h>
#include <memory>

#include "IO_Manager.h"
#include "TestClass.h"

#include "Computer_flops.h"



namespace ComputerFlopsTests
{
  using namespace network_butcher_types;
  using namespace network_butcher_computer;


  TEST(CompiterFlopsTests, ComputeFlopsGraph)
  {
    std::string const path  = "test_data/models/resnet18-v2-7-inferred.onnx";
    auto              graph = std::get<0>(network_butcher_io::IO_Manager::import_from_onnx(path, false, true, 3, true));

    for (auto const &node : graph.get_nodes())
      {
        auto const idk = Computer_flops::compute_macs_flops(node.content);
        std::cout << std::endl;
      }

    std::cout << std::endl;
  }

} // namespace ComputerFlopsTests