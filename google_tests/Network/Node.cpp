//
// Created by faccus on 13/09/21.
//

#include <gtest/gtest.h>
#include <memory>

#include "../../include/Network/Node.h"
#include "../../include/Traits/Node_traits.h"

using namespace network_butcher_types;

TEST(NodeTests, Conctructor)
{
  Node node(1);
}

TEST(NodeTests, MoveConstructible) {
  ASSERT_TRUE(std::is_move_constructible_v<node_type>);
}

TEST(NodeTests, MoveAssignable) {
  ASSERT_TRUE(std::is_move_assignable_v<node_type>);
}

