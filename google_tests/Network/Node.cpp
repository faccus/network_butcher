//
// Created by faccus on 13/09/21.
//

#include <gtest/gtest.h>
#include <memory>

#include "../../src/Network/Node.h"

TEST(NodeTests, ConctructorTest) {
  Node node(1, {1, 2, 3},
            {3, 4, 5}, {6, 7, 8});
}