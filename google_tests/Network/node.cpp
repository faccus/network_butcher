#include <gtest/gtest.h>
#include <memory>

#include <network_butcher/Network/node.h>
#include <network_butcher/Network/node_traits.h>


namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;

  TEST(NodeTests, Conctructor)
  {
    Node node;

    CNode<int> node_2(1);
  }

  TEST(NodeTests, MoveConstructible)
  {
    ASSERT_TRUE(std::is_move_constructible_v<Onnx_Converted_Node_Type>);
  }

  TEST(NodeTests, MoveAssignable)
  {
    ASSERT_TRUE(std::is_move_assignable_v<Onnx_Converted_Node_Type>);
  }
} // namespace