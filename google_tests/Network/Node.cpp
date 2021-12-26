//
// Created by faccus on 13/09/21.
//

#include <gtest/gtest.h>
#include <memory>

#include "../../src/Network/Node.h"

TEST(NodeTests, ConctructorTest)
{
  io_id_collection_type map;
  map["in"] = 10;


  Node node(map, map, map);
}